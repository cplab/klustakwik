/*
 * io.cpp
 *
 * Handles input and output to files.
 *
 *  Created on: 11 Nov 2011
 *      Author: dan
 */

// Disable some Visual Studio warnings
#define _CRT_SECURE_NO_WARNINGS

#include "klustakwik.h"
#include "numerics.h"

// Loads in Fet file.  Also allocates storage for other arrays
void KK::LoadData(char *FileBase, integer ElecNo, char *UseFeatures)
{
    char fname[STRLEN];
    //char fnamemask[STRLEN];
    char fnamefmask[STRLEN];
    char line[STRLEN];
    integer p, i, j;
	// nFeatures is read as a %d so it has to be int type, not integer type
    int nFeatures, nmaskFeatures; // not the same as nDims! we don't use all features.
    FILE *fp;
    //FILE *fpmask;
    FILE *fpfmask;
    integer status;
    //integer maskstatus;
    scalar val;
    //int maskval; // use int rather than integer because it is read as %d
    integer UseLen;
    scalar max, min;
    //bool usemasks = (UseDistributional && !UseFloatMasks);

    // open file
    sprintf(fname, "%s.fet.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "r");
    //if(usemasks)
    //{
    //    sprintf(fnamemask,"%s.mask.%d", FileBase, (int)ElecNo);
    //    fpmask = fopen_safe(fnamemask, "r");
    //} else
    //{
    //    fpmask = NULL;
    //}
    
    if((MaskStarts > 0)&& UseDistributional)
    {
        Output("-------------------------------------------------------------------------");
        Output("\nUsing Distributional EM with Maskstarts\n");
        MinClusters = MaskStarts;
        MaxClusters = MaskStarts;
        Output("NOTE: Maskstarts overides above values of MinClusters and MaxClusters \
                \nMinClusters = %d \nMaxClusters = %d \n ", (int)MinClusters, (int)MaxClusters);
    }
    
    if(UseDistributional)// replaces if(UseFloatMasks)
    {
        sprintf(fnamefmask,"%s.fmask.%d", FileBase, (int)ElecNo);
        fpfmask = fopen_safe(fnamefmask, "r");
    }
    else
    {
        fpfmask = NULL;
    }

    // count lines;
    nPoints=-1; // subtract 1 because first line is number of features
    while(fgets(line, STRLEN, fp)) {
        nPoints++;
    }

    // rewind file
    fseek(fp, 0, SEEK_SET);

    // read in number of features
    fscanf(fp, "%d", &nFeatures);
    if(Debug) Output("Number of features read in: %d \n ", nFeatures);

    // calculate number of dimensions
    if (UseFeatures[0] == 0)
    {
        nDims = nFeatures-DropLastNFeatures; // Use all but the last N Features.
        UseLen = nFeatures-DropLastNFeatures;
       // Output("nDims = %d ,UseLen = %d ", (int)nDims, (int)UseLen);
      //  UseFeatures =
    }
    else
    {
        UseLen = strlen(UseFeatures);
        nDims=0;
        for(i=0; i<nFeatures; i++)
        {
            nDims += (i<UseLen && UseFeatures[i]=='1');
        }
      //  Output("nDims = %d ,UseLen = %d ", (int)nDims, (int)UseLen);
    }
    nDims2 = nDims*nDims;
	MemoryCheck();
    AllocateArrays();

    // load data
    for (p=0; p<nPoints; p++) {
        j=0;
        for(i=0; i<nFeatures; i++) {
            float readfloatval;
            //status = fscanf(fp, SCALARFMT, &val);
            status = fscanf(fp, "%f", &readfloatval);
            val = (scalar)readfloatval;
            if (status==EOF) Error("Error reading feature file");

            if (UseFeatures[0] == 0) //when we want all the features
            {
                if(i<UseLen  ) //
                {
                    //        Output("j = %d, i = %d \n", (int)i, (int)j);
                    Data[p*nDims + j] = val;
               //             printf("Data = " SCALARFMT "",Data[p*nDims+j]);
                j++;
                }
            }
            else  // When we want the subset specified by the binary string UseFeatures, e.g. 111000111010101
            {
                if(i<UseLen && UseFeatures[i]=='1' ) 
                {
                    //     Output("j = %d, i = %d \n", (int)i, (int)j);
                    Data[p*nDims + j] = val;
                //               printf("Data = " SCALARFMT "",Data[p*nDims+j]);
                    j++;
                }
            }
        }
    }

    //if(usemasks)
    //{
//        // rewind file
//        fseek(fpmask, 0, SEEK_SET);

//        // read in number of features
//        fscanf(fpmask, "%d", &nmaskFeatures);

//        if (nFeatures != nmaskFeatures)
//            Error("Error: Mask file and Fet file incompatible");

//        // load masks
//        for (p=0; p<nPoints; p++) {
//            j=0;
//            for(i=0; i<nFeatures; i++) {
//                maskstatus = fscanf(fpmask, "%d", &maskval);
//                if (maskstatus==EOF) Error("Error reading mask file");

//                if (i<UseLen && UseFeatures[i]=='1') {
//                    Masks[p*nDims + j] = maskval;
//                    j++;
//                }
//            }
//        }
//    }
//    else  //Case for Classical KlustaKwik
//    {
//        for(p=0; p<nPoints; p++)
//            for(i=0; i<nDims; i++)
//                Masks[p*nDims+i] = 1;
//    }

    if(UseDistributional) //replaces if(UseFloatMasks)
    {
        // rewind file
        fseek(fpfmask, 0, SEEK_SET);

        // read in number of features
        fscanf(fpfmask, "%d", &nmaskFeatures);

        if (nFeatures != nmaskFeatures)
            Error("Error: Float Mask file and Fet file incompatible");

        // load float masks
        for (p=0; p<nPoints; p++) {
            j=0;
            for(i=0; i<nFeatures; i++)
            {
                float readfloatval;
                status = fscanf(fpfmask, "%f", &readfloatval);
                if (status==EOF) Error("Error reading fmask file");
                val = (scalar)readfloatval;

                if (UseFeatures[0] == 0)
                {
                    if(i<UseLen )
                    {
                      //                                  Output("j = %d, i = %d \n", (int)i, (int)j);
                        FloatMasks[p*nDims + j] = val;
                     //                                   printf("Data = " SCALARFMT "",Data[p*nDims+j]);
                        j++;
                    }
                }
                else  // When we want all the features
                {
                    if(i<UseLen && UseFeatures[i]=='1'  ) //To Do: implement DropLastNFeatures
                    {
                        FloatMasks[p*nDims + j] = val;
                      //                                 printf("Data = " SCALARFMT "",Data[p*nDims+j]);
                        j++;
                    }
                }
               // if (i<UseLen && UseFeatures[i]=='1') {
               //     FloatMasks[p*nDims + j] = val;
               //     j++;
               // }
            }
        }
    }
    //else if(UseDistributional)
    //{
    //    for(p=0; p<nPoints; p++)
    //        for(i=0; i<nDims; i++)
    //        {
    //            FloatMasks[p*nDims+i] = (scalar)Masks[p*nDims+i];
    //        }
    //}
    

    if(UseDistributional)
    {
        for(p=0; p<nPoints; p++)
            for(i=0; i<nDims; i++)
            {
                if(FloatMasks[p*nDims+i]==(scalar)1) //changed so that this gives the connected component masks
                    Masks[p*nDims+i] = 1;
                else
                    Masks[p*nDims+i] = 0;
            }
    }
    else  //Case for Classical EM KlustaKwik
    {
        for(p=0; p<nPoints; p++)
            for(i=0; i<nDims; i++)
                Masks[p*nDims+i] = 1;
    }

    fclose(fp);
    //if(usemasks)
    //    fclose(fpmask);
    if(UseDistributional)
        fclose(fpfmask);

    // normalize data so that range is 0 to 1: This is useful in case of v. large inputs
    for(i=0; i<nDims; i++) {

        //calculate min and max
        min = HugeScore; max=-HugeScore;
        for(p=0; p<nPoints; p++) {
            val = Data[p*nDims + i];
            if (val > max) max = val;
            if (val < min) min = val;
        }

        // now normalize
        for(p=0; p<nPoints; p++) Data[p*nDims+i] = (Data[p*nDims+i] - min) / (max-min);
    }

    Output("----------------------------------------------------------\nLoaded %d data points of dimension %d.\n", (int)nPoints, (int)nDims);
    Output("MEMO: A lower score indicates a better clustering \n ");
}

// write output to .clu file - with 1 added to cluster numbers, and empties removed.
void KK::SaveOutput()
{
    integer c;
    uinteger p;
    char fname[STRLEN];
    FILE *fp;
    integer MaxClass = 0;
    vector<integer> NotEmpty(MaxPossibleClusters);
    vector<integer> NewLabel(MaxPossibleClusters);

    // find non-empty clusters
    for(c=0;c<MaxPossibleClusters;c++) NewLabel[c] = NotEmpty[c] = 0;
    for(p=0; p<BestClass.size(); p++) NotEmpty[BestClass[p]] = 1;

    // make new cluster labels so we don't have empty ones
    NewLabel[0] = 1;
    MaxClass = 1;
    for(c=1;c<MaxPossibleClusters;c++) {
        if (NotEmpty[c]) {
            MaxClass++;
            NewLabel[c] = MaxClass;
        }
    }

    // print file
    sprintf(fname, "%s.clu.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "w");

    fprintf(fp, "%d\n", (int)MaxClass);
    for (p=0; p<BestClass.size(); p++) fprintf(fp, "%d\n", (int)NewLabel[BestClass[p]]);

    fclose(fp);

    if(SaveCovarianceMeans)
        SaveCovMeans();
    if(SaveSorted&&UseDistributional)
        SaveSortedClu();
}

// write output to .clu file - with 1 added to cluster numbers, and empties removed.
void KK::SaveTempOutput()
{
    integer c;
    uinteger p;
    char fname[STRLEN];
    FILE *fp;
    integer MaxClass = 0;
    vector<integer> NotEmpty(MaxPossibleClusters);
    vector<integer> NewLabel(MaxPossibleClusters);
    
    // find non-empty clusters
    for(c=0;c<MaxPossibleClusters;c++) NewLabel[c] = NotEmpty[c] = 0;
    for(p=0; p<BestClass.size(); p++) NotEmpty[BestClass[p]] = 1;
    
    // make new cluster labels so we don't have empty ones
    NewLabel[0] = 1;
    MaxClass = 1;
    for(c=1;c<MaxPossibleClusters;c++) {
        if (NotEmpty[c]) {
            MaxClass++;
            NewLabel[c] = MaxClass;
        }
    }
    
    // print file
    sprintf(fname, "%s.temp.clu.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "w");
    
    fprintf(fp, "%d\n", (int)MaxClass);
    for (p=0; p<BestClass.size(); p++) fprintf(fp, "%d\n", (int)NewLabel[BestClass[p]]);
    
    fclose(fp);
    
    if(SaveCovarianceMeans)
        SaveCovMeans();
    if(SaveSorted&&UseDistributional)
        SaveSortedClu();
}

void KK::SaveCovMeans()
{
    char fname[STRLEN];
    FILE *fp;
    // print covariance to file
    sprintf(fname, "%s.cov.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "w");
    for (integer cc=0; cc<nClustersAlive; cc++)
    {
        integer c = AliveIndex[cc];
        for(integer i=0; i<nDims; i++)
        {
            for(integer j=0; j<nDims; j++)
            {
				// TODO: update Cov output for distributional
				if (!UseDistributional)
					fprintf(fp, SCALARFMT " ", Cov[c*nDims2+i*nDims+j]);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    // print mean to file
    sprintf(fname, "%s.mean.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "w");
    for (integer cc=0; cc<nClustersAlive; cc++)
    {
        integer c = AliveIndex[cc];
        for(integer i=0; i<nDims; i++)
        {
            fprintf(fp, SCALARFMT " ", Mean[c*nDims+i]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

// Saves sorted.fet and sorted.mask file
void KK::SaveSortedData()
{
    char fname[STRLEN];
    FILE *fp;
    // sorted.fet file
    sprintf(fname, "%s.sorted.fet.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "w");
    fprintf(fp, "%d\n", (int)nDims);
    for(integer q=0; q<nPoints; q++)
    {
        integer p = SortedIndices[q];
        for(integer i=0; i<nDims; i++)
            fprintf(fp, SCALARFMT " ", Data[p*nDims+i]);
        fprintf(fp, "\n");
    }
    fclose(fp);
    // sorted.mask file
    sprintf(fname, "%s.sorted.mask.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "w");
    fprintf(fp, "%d\n", (int)nDims);
    for(integer q=0; q<nPoints; q++)
    {
        integer p = SortedIndices[q];
        for(integer i=0; i<nDims; i++)
            fprintf(fp, "%d ", (int)Masks[p*nDims+i]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

// Save sorted.clu file (see SaveOutput for explanation)
void KK::SaveSortedClu()
{
    char fname[STRLEN];
    FILE *fp;
    vector<integer> NotEmpty(MaxPossibleClusters);
    vector<integer> NewLabel(MaxPossibleClusters);
    for(integer c=0; c<MaxPossibleClusters; c++)
        NewLabel[c] = NotEmpty[c] = 0;
    for(integer q=0; q<nPoints; q++)
        NotEmpty[Class[SortedIndices[q]]] = 1;
    NewLabel[0] = 1;
    integer MaxClass = 1;
    for(integer c=1; c<MaxPossibleClusters; c++)
        if(NotEmpty[c])
            NewLabel[c] = ++MaxClass;
    sprintf(fname, "%s.sorted.clu.%d", FileBase, (int)ElecNo);
    fp = fopen_safe(fname, "w");
    fprintf(fp, "%d\n", (int)MaxClass);
    for(integer q=0; q<nPoints; q++)
        fprintf(fp, "%d\n", (int)NewLabel[Class[SortedIndices[q]]]);
    fclose(fp);
}
