#include "sdcard.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

static bool isOpen = false;

inline static void swap(char **a, char **b)
{
    char *t = *a;
    *a = *b;
    *b = t;
}

static int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++)
    {
        int d = tolower((int)*a) - tolower((int)*b);
        if (d != 0 || !*a)
            return d;
    }
}

static int partition(char *arr[], int low, int high)
{
    char *pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++)
    {
        if (strcicmp(arr[j], pivot) < 0)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

static void quick_sort(char *arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition(arr, low, high);

        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

static void sort_files(char **files, int count)
{
    if (count > 1)
    {
        quick_sort(files, 0, count - 1);
    }
}

int sdcard_get_files_count(const char *path)
{
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir(path); /* There should be error handling after this */
    if (dirp == NULL)
    {
        printf("opendir failed.\n");
        return 0;
    }
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            file_count++;
        }
    }
    closedir(dirp);

    return file_count;
}

int sdcard_files_get(const char *path, const char *extension, char ***filesOut)
{
    const int MAX_FILES = 2048;

    int count = 0;
    char **result = (char **)malloc(MAX_FILES * sizeof(void *));
    if (!result)
        abort();

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        printf("opendir failed.\n");
        //abort();
        return 0;
    }

    int extensionLength = strlen(extension);
    if (extensionLength < 1)
        abort();

    char *temp = (char *)malloc(extensionLength + 1);
    if (!temp)
        abort();

    memset(temp, 0, extensionLength + 1);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        size_t len = strlen(entry->d_name);

        // ignore 'hidden' files (MAC)
        bool skip = false;
        if (entry->d_name[0] == '.')
            skip = true;

        memset(temp, 0, extensionLength + 1);
        if (!skip)
        {
            for (int i = 0; i < extensionLength; ++i)
            {
                temp[i] = tolower((int)entry->d_name[len - extensionLength + i]);
            }

            if (len > extensionLength)
            {
                if (strcmp(temp, extension) == 0)
                {
                    result[count] = (char *)malloc(len + 1);
                    //printf("%s: allocated %p\n", __func__, result[count]);

                    if (!result[count])
                    {
                        abort();
                    }

                    strcpy(result[count], entry->d_name);
                    ++count;

                    if (count >= MAX_FILES)
                        break;
                }
            }
        }
    }

    closedir(dir);
    free(temp);

    sort_files(result, count);

    *filesOut = result;
    return count;
}

void sdcard_files_free(char **files, int count)
{
    for (int i = 0; i < count; ++i)
    {
        //printf("%s: freeing item %p\n", __func__, files[i]);
        free(files[i]);
    }

    //printf("%s: freeing array %p\n", __func__, files);
    free(files);
}

esp_err_t sdcard_open(const char *base_path)
{
    esp_err_t ret;

    if (isOpen)
    {
        printf("sdcard_open: alread open.\n");
        ret = ESP_FAIL;
    }
    else
    {
        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        host.flags = SDMMC_HOST_FLAG_1BIT;
//        host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

        sdmmc_slot_config_t slot_config = {
			    .width = 1, .flags = 0,
			    .d0 = 17, .d1 = -1, .d2 = -1, .d3 = -1, .clk = 21, .cmd = 14,
			    .d4 = -1, .d5 = -1, .d6 = -1, .d7 = -1, .cd = -1, .wp = -1,
	    };
        // Options for mounting the filesystem.
        // If format_if_mount_failed is set to true, SD card will be partitioned and
        // formatted in case when mounting fails.
	    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
			    .format_if_mount_failed = false,
			    .max_files = 5,
			    .allocation_unit_size = 32 * 1024
	    };
        // Use settings defined above to initialize SD card and mount FAT filesystem.
        // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
        // Please check its source code and implement error recovery when developing
        // production applications.
        sdmmc_card_t *card;
        ret = esp_vfs_fat_sdmmc_mount(base_path, &host, &slot_config, &mount_config, &card);

        if (ret == ESP_OK)
        {
            isOpen = true;
        }
        else
        {
            printf("sdcard_open: esp_vfs_fat_sdmmc_mount failed (%d)\n", ret);
        }
    }

    return ret;
}

esp_err_t sdcard_close()
{
    esp_err_t ret;

    if (!isOpen)
    {
        printf("sdcard_close: not open.\n");
        ret = ESP_FAIL;
    }
    else
    {
        ret = esp_vfs_fat_sdmmc_unmount();

        if (ret != ESP_OK)
        {
            printf("sdcard_close: esp_vfs_fat_sdmmc_unmount failed (%d)\n", ret);
        }
        else
        {
            isOpen = false;
        }
    }

    return ret;
}

size_t sdcard_get_filesize(const char *path)
{
    size_t ret = 0;

    if (!isOpen)
    {
        printf("sdcard_get_filesize: not open.\n");
    }
    else
    {
        FILE *f = fopen(path, "rb");
        if (f == NULL)
        {
            printf("sdcard_get_filesize: fopen failed.\n");
        }
        else
        {
            // get the file size
            fseek(f, 0, SEEK_END);
            ret = ftell(f);
            fseek(f, 0, SEEK_SET);
        }
    }

    return ret;
}

static char* ROM_DATA = (char*)0x3D000000;

char *osd_getromdata()
{

    printf("Initialized. ROM@%p\n", ROM_DATA);
    return (char*)ROM_DATA;
}

size_t sdcard_copy_file_to_memory(const char *path)
{
    size_t ret = 0;

    if (!isOpen)
    {
        printf("sdcard_copy_file_to_memory: not open.\n");
    }
    else
    {
	    FILE *f = fopen(path, "rb");
	    if (f == NULL)
	    {
		    printf("sdcard_copy_file_to_memory: fopen failed.\n");
	    }
	    else
	    {
		    printf("sdcard_copy_file_to_memory: read before free heap: %d kb,%d mb\n",esp_get_free_heap_size()/1024,esp_get_free_heap_size()/1024/1024);
		    printf("sdcard_copy_file_to_memory: read before free internal heap: %d kb\n",esp_get_free_internal_heap_size()/1024);
		    //get size
		    fseek(f,0L,SEEK_END);
		    long romSize = ftell(f);
		    rewind(f);
		    printf("sdcard_copy_file_to_memory: file size: %ld",romSize/1024);
		    ROM_DATA = malloc(sizeof(uint8_t)*romSize);
		    if (ROM_DATA == NULL)
		    {
			    printf("sdcard_copy_file_to_memory: malloc mem fail");
			    abort();
		    }
		    memset(ROM_DATA,0,sizeof(uint8_t)*romSize);
		    printf("sdcard_copy_file_to_memory: malloc mem size:%ld kb，address:%#x.\n",sizeof(uint8_t)*romSize/1024,(size_t)ROM_DATA);
		    printf("sdcard_copy_file_to_memory: free heap: %d kb,%d mb\n",esp_get_free_heap_size()/1024,esp_get_free_heap_size()/1024/1024);
		    printf("sdcard_copy_file_to_memory: free internal heap: %d kb\n",esp_get_free_internal_heap_size()/1024);
		    // copy
		    const size_t BLOCK_SIZE = 512;
		    while (true)
		    {
//	                printf("sdcard_copy_file_to_memory: copy addr: %X ret: %X\n",((int)ramPtr+ret),ret);
//                    __asm__("memw");
			    size_t count = fread((uint8_t *)ROM_DATA + ret, 1, BLOCK_SIZE, f);
//                    __asm__("memw");
			    ret += count;
			    if (count < BLOCK_SIZE)
				    break;
		    }
		    printf("sdcard_copy_file_to_memory: copy done\n");
		    printf("osd_getromdata: copy %s file to memory %X.\n",path,(int)ROM_DATA);
	    }
    }

    return ret;
}

char *sdcard_create_savefile_path(const char *base_path, const char *fileName)
{
    char *result = NULL;

    if (!base_path)
        abort();
    if (!fileName)
        abort();

    //printf("%s: base_path='%s', fileName='%s'\n", __func__, base_path, fileName);

    // Determine folder
    char *extension = fileName + strlen(fileName); // place at NULL terminator
    while (extension != fileName)
    {
        if (*extension == '.')
        {
            ++extension;
            break;
        }
        --extension;
    }

    if (extension == fileName)
    {
        printf("%s: File extention not found.\n", __func__);
        abort();
    }

    //printf("%s: extension='%s'\n", __func__, extension);

    const char *DATA_PATH = "/esplay/data/";
    const char *SAVE_EXTENSION = ".sav";

    size_t savePathLength = strlen(base_path) + strlen(DATA_PATH) + strlen(extension) + 1 + strlen(fileName) + strlen(SAVE_EXTENSION) + 1;
    char *savePath = malloc(savePathLength);
    if (savePath)
    {
        strcpy(savePath, base_path);
        strcat(savePath, DATA_PATH);
        strcat(savePath, extension);
        strcat(savePath, "/");
        strcat(savePath, fileName);
        strcat(savePath, SAVE_EXTENSION);

        printf("%s: savefile_path='%s'\n", __func__, savePath);

        result = savePath;
    }

    return result;
}
