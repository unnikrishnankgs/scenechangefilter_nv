#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <dirent.h> 

using namespace cv;
using namespace std;

#define printf(...)

//#define TEST_SAMPLE_FILES

#define THRESHOLD_NV_CAM (3.8)
#define THRESHOLD_IOWA_CAM (1.0)
#define THRESHOLD THRESHOLD_IOWA_CAM

/**
 * @param apcImg1[IN] path to the file (img1)
 * @param apcImg2[IN] path to the file (img2)
 * @return 0 if same (or very minor differences; specific to NVIDIA AI City 
 *                   challange req)
 *         1 if different (with substantial scenechange)
 */
int compareImages(char* apcImg1, char* apcImg2)
{
    /** Mat is a n-dimentional single/multi (color) channel array storage data-structure */
    Mat image1;
    Mat image2;
    #ifdef VISUALIZE_DIFF
    Mat diff;
    #endif /**< VISUALIZE_DIFF */
    unsigned long long i, j;
    /** sum of absolute differences */
    double wa = 0; /**< weighted-avg */
    double waB = 0; /**< weighted-avg */
    double waG = 0; /**< weighted-avg */
    double waR = 0; /**< weighted-avg */
    vector<int> compression_params;


    if(!apcImg1 || !apcImg2)
    {
        return 0;
    }
    printf("comparing [%s] and [%s]\n", apcImg1, apcImg2);
    /** imread is implemented in loadsave.cpp; the functions to load IPL (Image Processing Library)
     * images into memory sits here
     */
    namedWindow("displayimgLL", CV_WINDOW_AUTOSIZE);
    image1 = imread(apcImg1, IMREAD_COLOR);
    image2 = imread(apcImg2, IMREAD_COLOR);
    if(!image1.data || !image2.data || !image1.total() 
        || (image1.total() != image2.total() || image1.rows != image2.rows) /**< ensure both images are same dimentions */
      )
    {
        printf("No image data!\n");
        return -1;
    }

    #ifdef VISUALIZE_DIFF
    absdiff(image1, image2, diff);
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(100);
    imwrite("diff.jpeg", diff, compression_params);
    printf("depth of the diff image is = %d channels=%d\n", diff.depth(), diff.channels());
    #endif /**< VISUALIZE_DIFF */
    for(i = 0; i < image1.rows; i++)
    {
        for(j = 0; j < image2.cols; j++)
        {
            waB += abs(image1.data[image1.channels()*(image1.cols*i + j) + 0] -  image2.data[image1.channels()*(image2.cols*i + j) + 0]);
            waG += abs(image1.data[image1.channels()*(image1.cols*i + j) + 1] -  image2.data[image1.channels()*(image2.cols*i + j) + 1]);
            waR += abs(image1.data[image1.channels()*(image1.cols*i + j) + 2] -  image2.data[image1.channels()*(image2.cols*i + j) + 2]);
        }
    }
    wa = (waB + waG + waR) / 3;
    wa /= image1.total();
    waB /= image1.total();
    waG /= image1.total();
    waR /= image1.total();
    printf("wa = %lf waB=%lf waG=%lf waR=%lf\n", wa, waB, waB, waR);

    if(wa < THRESHOLD)
    {
        return 0;
    }

    return 1; /**< substantial diff in scene */
}

typedef struct
{
    char* pcF;
    bool isRemoved;
}tJPEGFile;

#define MV_DIR_NAME "removed_files"
#define MV_CMD "mv \"%s\" \"%s\"/"


int compareJPEGImages(tJPEGFile* apJPG1, tJPEGFile* apJPG2)
{
    int ret = 0;
    char* pcMvCmd;
    int n = 0;
    int nC = 0;
    ret = compareImages(apJPG1->pcF, apJPG2->pcF);
    if(ret == 0)
    {
        apJPG1->isRemoved = true;
        n = strlen(apJPG1->pcF) + strlen(MV_CMD) + strlen(MV_DIR_NAME);
        pcMvCmd = (char*)malloc((n + 1) * sizeof(char));
        nC = sprintf(pcMvCmd, MV_CMD, apJPG1->pcF, MV_DIR_NAME);
        printf("nC=%d n=%d\n", nC, n);
        if(nC)
        {
            system(pcMvCmd);
        }
    }
    return ret;
}

int main(int argc, char* argv[])
{
    DIR* d;
    struct dirent *dir;
    tJPEGFile* pJPGFiles;
    unsigned long long nJPGFiles = 0;
    unsigned long long nIdxJPGFiles = 0;
    unsigned long long i = 0;
    unsigned long long j = 0;

#ifdef TEST_SAMPLE_FILES
    if(argc < 3)
    {
        printf("usage: $displayimg image_path\n");
        return -1;
    }

    int diff;
    diff = compareImages(argv[1], argv[2]);
    printf("the images are %s\n", diff ? "diff" : "similar");
#endif

    d = opendir(argv[1]);
    if(d)
    {
        while((dir = readdir(d)) != NULL)
        {
            if(strstr(dir->d_name, "jpeg"))
            {
                nJPGFiles++;
            }
        }
        closedir(d);
    }

    if(!nJPGFiles)
        return -1;

    pJPGFiles = (tJPEGFile*)calloc(1, sizeof(tJPEGFile) * nJPGFiles);

    d = opendir(argv[1]);
    if(d)
    {
        while((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
            if(strstr(dir->d_name, "jpeg"))
            {
                printf("is jpeg file\n");
                int n = strlen(dir->d_name) + 1; /**< + NULL chara */
                printf("n = %d\n", n);

                if(n)
                {
                    pJPGFiles[nIdxJPGFiles].pcF = (char*)malloc(n * sizeof(char));
                    strcpy(pJPGFiles[nIdxJPGFiles].pcF, dir->d_name);
                    nIdxJPGFiles++;
                }
            }
        }
        closedir(d);
    }

    if(nJPGFiles <= 1)
        return -1;

    for (i = 0; i < nJPGFiles - 1 ; i++)
    {
        for (j = i + 1; j < nJPGFiles; j++)
        {
            if (strcmp(pJPGFiles[i].pcF, pJPGFiles[j].pcF) > 0)
            {
                char* temp = (char*)malloc(strlen(pJPGFiles[i].pcF) + 1);
                strcpy(temp, pJPGFiles[i].pcF);
                strcpy(pJPGFiles[i].pcF, pJPGFiles[j].pcF);
                strcpy(pJPGFiles[j].pcF, temp);
            }
        }
    }
    
    for (i = 0; i < nJPGFiles; i++)
    {
        printf("%s\n", pJPGFiles[i].pcF);
    }

    system("mkdir " MV_DIR_NAME);

    for(i = 1; i < nIdxJPGFiles; i++)
    {
        compareJPEGImages(&pJPGFiles[i - 1], &pJPGFiles[i]);
    }
    return 0;
}
