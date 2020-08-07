#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

char Command[256];

typedef struct log
{
    char log_operation_type[8];
    char log_type;
    char log_name[96];
    char log_content[128];
    char log_date[24];
    char log_operation[256];
}log;

typedef struct block
{
    char block_data[25];
}block;

typedef struct inode
{
    int i_count;
    int i_file_size;
    int i_block_count;
    int i_p_block[4];
    int i_p_i;
    int i_proprietary;
}inode;

typedef struct dd_f
{
    char dd_file_name[16];
    int dd_file_p_inode;
    char dd_file_creation_date[24];
    char dd_file_modified_date[24];
}dd_f;

typedef struct dd
{
    dd_f files[5];
    int dd_p_dd;
}dd;

typedef struct tvd
{
    char tvd_creation_date[24];
    char tvd_directory_name[16];
    int tvd_p_subdirectorys[6];
    int tvd_p_detail_directory;
    int tvd_p_tvd;
    int tvd_proprietary;
}tvd;

typedef struct sb
{
    char sb_disk_name[16];
    int sb_virtual_tree_count;
    int sb_detail_directory_count;
    int sb_inode_count;
    int sb_block_count;
    int sb_virtual_tree_free;
    int sb_detail_directory_free;
    int sb_inode_free;
    int sb_block_free;
    char sb_date_creation[24];
    char sb_date_last_montage[24];
    int sb_montage_count;
    int sb_p_tree_directory_bitmap;
    int sb_p_tree_directory;
    int sb_p_detail_directory_bitmap;
    int sb_p_detail_directory;
    int sb_p_inode_bitmap;
    int sb_p_inode;
    int sb_p_block_bitmap;
    int sb_p_block;
    int sb_p_log;
    int sb_p_backup;
    int sb_s_tree_directory;
    int sb_s_detail_directory;
    int sb_s_inode;
    int sb_s_block;
    int sb_ffb_tree_directory;
    int sb_ffb_detail_directory;
    int sb_ffb_inode;
    int sb_ffb_block;
    int sb_magic_number;
}sb;

typedef struct vd
{
    char active;
    char path[96];
    char name[16];
    char letter;
    int number;
}vd;

vd MountedPartitions[256];

typedef struct ebr
{
    char part_status;
    char part_fit;
    int part_start;
    int part_size;
    int part_next;
    char part_name[16];
}ebr;

typedef struct partition
{
    char part_status;
    char part_type;
    char part_fit;
    int part_start;
    int part_size;
    char part_name[16];
}partition;

typedef struct mbr
{
    int mbr_size;
    time_t mbr_creation_date;
    int mbr_disk_signature;
    partition mbr_partition_1;
    partition mbr_partition_2;
    partition mbr_partition_3;
    partition mbr_partition_4;
}mbr;

typedef struct rep
{
    char name[14];
    char path[96];
    char ruta[96];
    char id[6];
}rep;

typedef struct _mkfile
{
    char id[6];
    char path[96];
    char p;
    int size;
    char content[128];
}_mkfile;

typedef struct _mkdir
{
    char id[6];
    char path[96];
    char p;
}_mkdir;

typedef struct mkfs
{
    char id[6];
    char type[5];
}mkfs;

typedef struct unmount
{
    char name[16];
}unmount;

typedef struct mount
{
    char path[96];
    char name[16];
}mount;

typedef struct fdisk
{
    int size;
    char unit;
    char path[96];
    char type;
    char fit[2];
    char _delete[5];
    char name[16];
    int add;
}fdisk;

typedef struct rmdisk
{
    char path[96];
}rmdisk;

typedef struct mkdisk
{
    int size;
    char path[96];
    char name[32];
    char unit;
}mkdisk;

int compareArray(int CommandIndex, char* Array, int ArrayLength)
{
    int MatchCounter = 0;
    int j = 0;
    for(int i = CommandIndex; i < CommandIndex + ArrayLength; i++)
    {
        if(Command[i] == Array[j])
        {
            MatchCounter++;
            j++;
        }
    }
    if(MatchCounter == ArrayLength)
        return 1;
    return 0;
}

int itsNumber(char Character)
{
    for(int i = 0; i < 10; i++)
    {
        if(Character == i + '0')
            return 1;
    }
    return 0;
}

int itsIdentifier(char Character)
{
    for(int i = 0; i < 10; i++)
    {
        if(Character == i + '0')
            return 1;
    }
    for(int i = 65; i < 91; i++)
    {
        if(Character == i)
            return 1;
    }
    for(int i = 97; i < 123; i++)
    {
        if(Character == i)
            return 1;
    }
    if(Character == '_')
        return 1;
    return 0;
}

void createDirectory(char* Path,int PathLength)
{
    for(int i = 1; i < PathLength; i++)
    {
        if(Path[i] == '/')
        {
            char TemporalPath[i+1];
            for(int j = 0; j < i; j++)
            {
                TemporalPath[j] = Path[j];
            }
            TemporalPath[i] = '\0';
            printf(">> El directorio ");
            printf("%s",TemporalPath);
            if (mkdir(TemporalPath, 0777) == -1)
                printf(" ya existe.\n");
            else
                printf(" ha sido creado.\n");
        }
    }
}

int getFinalSize(int Magnitude, char Unit)
{
    if(Unit == 'k')
    {
        return Magnitude * 1024;
    }
    if(Unit == 'm')
    {
        return Magnitude * 1024 * 1024;
    }
    return 0;
}

int getTotalStructures(int PartitionSize)
{
    int SuperBlockSize = sizeof(sb);
    int VirtualTreeSize = sizeof(tvd);
    int DetailDirectorySize = sizeof(dd);
    int InodeSize = sizeof(inode);
    int BlockSize = sizeof(block);
    int LogSize = sizeof(log);
    return ((PartitionSize - (2 * SuperBlockSize))/
            (27 + VirtualTreeSize + DetailDirectorySize + (5*InodeSize+(20*BlockSize)+LogSize)));
}

void initializeLOG(log *LOG)
{
    memset((*LOG).log_operation_type,'\0',8);
    (*LOG).log_type = '\0';
    memset((*LOG).log_name,'\0',96);
    memset((*LOG).log_content,'\0',128);
    memset((*LOG).log_date,'\0',24);
    memset((*LOG).log_operation,'\0',256);
}

void initializeBLOCK(block *BLOCK)
{
    memset((*BLOCK).block_data,'\0',25);
}

void initializeINODE(inode *INODE)
{
    (*INODE).i_count = 0;
    (*INODE).i_file_size = 0;
    (*INODE).i_block_count = 0;
    for(int i = 0; i < 4; i++)
    {
        (*INODE).i_p_block[i] = -1;
    }
    (*INODE).i_p_i = -1;
    (*INODE).i_proprietary = 0;
}

void initializeDD_F(dd_f *DD_F)
{
    memset((*DD_F).dd_file_name,'\0',16);
    (*DD_F).dd_file_p_inode = 0;
    memset((*DD_F).dd_file_creation_date,'\0',24);
    memset((*DD_F).dd_file_modified_date,'\0',24);
}

void initializeDD(dd *DD)
{
    for(int i = 0; i < 5; i++)
    {
        initializeDD_F(&((*DD).files[i]));
    }
    (*DD).dd_p_dd = -1;
}

void initializeTVD(tvd *TVD)
{
    memset((*TVD).tvd_creation_date,'\0',24);
    memset((*TVD).tvd_directory_name,'\0',16);
    for(int i = 0; i < 6; i++)
    {
        (*TVD).tvd_p_subdirectorys[i] = -1;
    }
    (*TVD).tvd_p_detail_directory = -1;
    (*TVD).tvd_p_tvd = -1;
    (*TVD).tvd_proprietary = -1;
}

void initializeSB(sb *SB)
{
    memset((*SB).sb_disk_name,'\0',16);
    (*SB).sb_virtual_tree_count = 0;
    (*SB).sb_detail_directory_count = 0;
    (*SB).sb_inode_count = 0;
    (*SB).sb_block_count = 0;
    (*SB).sb_virtual_tree_free = 0;
    (*SB).sb_detail_directory_free = 0;
    (*SB).sb_inode_free = 0;
    (*SB).sb_block_free = 0;
    memset((*SB).sb_date_creation,'\0',24);
    memset((*SB).sb_date_last_montage,'\0',24);
    (*SB).sb_montage_count = 0;
    (*SB).sb_p_tree_directory_bitmap = 0;
    (*SB).sb_p_tree_directory = 0;
    (*SB).sb_p_detail_directory_bitmap = 0;
    (*SB).sb_p_detail_directory = 0;
    (*SB).sb_p_inode_bitmap = 0;
    (*SB).sb_p_inode = 0;
    (*SB).sb_p_block_bitmap = 0;
    (*SB).sb_p_block = 0;
    (*SB).sb_p_log = 0;
    (*SB).sb_p_backup = 0;
    (*SB).sb_s_tree_directory = 0;
    (*SB).sb_s_detail_directory = 0;
    (*SB).sb_s_inode = 0;
    (*SB).sb_s_block = 0;
    (*SB).sb_ffb_tree_directory = 0;
    (*SB).sb_ffb_detail_directory = 0;
    (*SB).sb_ffb_inode = 0;
    (*SB).sb_ffb_block = 0;
    (*SB).sb_magic_number = 0;
}

void writeLSDirectory(char *Path, int VirtualTreePointer, sb *SB, FILE *Disk)
{
    int EndDirectory = -1;
    int SlashCounter = 0;
    int FatherDirectory = 0;
    for(int i = 0; i < 96; i++)
    {
        if(Path[i] == '/')
        {
            SlashCounter++;
            if(SlashCounter == 2)
            {
                FatherDirectory = 1;
                EndDirectory = i;
                i = 96;
            }
        }
    }
    if(EndDirectory == -1)
    {
        for(int i = 95; i >= 0; i--)
        {
            if(Path[i] != '\0')
            {
                EndDirectory = i + 1;
                i = -1;
            }
        }
    }
    char Directory[16];
    memset(Directory,'\0',16);
    int j = 0;
    for(int i = 1; i < EndDirectory; i++)
    {
        Directory[j] = Path[i];
        j++;
    }
    if(FatherDirectory == 1)
    {
        tvd TVD;
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        int DirectoryFounded = -1;
        int SearchingDirectory = 1;
        do{
            for(int i = 0; i < 6; i++)
            {
                if(TVD.tvd_p_subdirectorys[i] != -1)
                {
                    tvd TemporalTVD;
                    initializeTVD(&TemporalTVD);
                    fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                    fread(&TemporalTVD,sizeof(tvd),1,Disk);
                    int NameMatchCounter = 0;
                    for(int j = 0; j < 16; j++)
                    {
                        if(Directory[j] == TemporalTVD.tvd_directory_name[j])
                            NameMatchCounter++;
                    }
                    if(NameMatchCounter == 16)
                    {
                        DirectoryFounded = TVD.tvd_p_subdirectorys[i];
                    }
                }
            }
            if(DirectoryFounded == -1)
            {
                if(TVD.tvd_p_tvd != -1)
                {
                    VirtualTreePointer = TVD.tvd_p_tvd;
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                }
                else
                {
                    SearchingDirectory = 0;
                }
            }
            else
            {
                SearchingDirectory = 0;
            }
        }while(SearchingDirectory);
        if(DirectoryFounded != -1)
        {
            char NewPath[96];
            memset(NewPath,'\0',96);
            int j = 0;
            for(int i = EndDirectory; i < 96; i++)
            {
                NewPath[j] = Path[i];
                j++;
            }
            writeLSDirectory(NewPath,DirectoryFounded,&(*SB),&(*Disk));
        }
    }
    else
    {
        tvd TVD;
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        int TotalDirectorys = 1;
        int ShowingDirectorys = 1;
        printf(">> Directorios disponibles:\n");
        do
        {
            for(int i = 0; i < 6; i++)
            {
                if(TVD.tvd_p_subdirectorys[i] != -1)
                {
                    tvd TemporalTVD;
                    initializeTVD(&TemporalTVD);
                    fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                    fread(&TemporalTVD,sizeof(tvd),1,Disk);
                    printf("%d. %s - %s - %d\n",TotalDirectorys,TemporalTVD.tvd_directory_name,
                           TemporalTVD.tvd_creation_date,TemporalTVD.tvd_proprietary);
                    TotalDirectorys++;
                }
            }
            if(TVD.tvd_p_tvd != -1)
            {
                fseek(Disk,TVD.tvd_p_tvd,SEEK_SET);
                fread(&TVD,sizeof(tvd),1,Disk);
            }
            else
            {
                ShowingDirectorys = 0;
            }
        }
        while(ShowingDirectorys);
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        int TotalFiles = 1;
        printf(">> Archivos disponibles:\n");
        if(TVD.tvd_p_detail_directory != -1)
        {
            dd DD;
            initializeDD(&DD);
            fseek(Disk,TVD.tvd_p_detail_directory,SEEK_SET);
            fread(&DD,sizeof(dd),1,Disk);
            int ShowingDetailFiles = 1;
            do
            {
                for(int i = 0; i < 5; i++)
                {
                    if(DD.files[i].dd_file_name[0] != '\0')
                    {
                        printf("%d. %s - %s\n",TotalFiles,DD.files[i].dd_file_name,DD.files[i].dd_file_creation_date);
                        TotalFiles++;
                    }
                }
                if(DD.dd_p_dd != -1)
                {
                    fseek(Disk,DD.dd_p_dd,SEEK_SET);
                    fread(&DD,sizeof(dd),1,Disk);
                }
                else
                {
                    ShowingDetailFiles = 0;
                }
            }while(ShowingDetailFiles);
        }
    }
}

void writeTreeFile(char *Path, int VirtualTreePointer, sb *SB, FILE *Disk, FILE *File, int *NodeNumber)
{
    int EndDirectory = -1;
    int SlashCounter = 0;
    int FatherDirectory = 0;
    for(int i = 0; i < 96; i++)
    {
        if(Path[i] == '/')
        {
            SlashCounter++;
            if(SlashCounter == 2)
            {
                FatherDirectory = 1;
                EndDirectory = i;
                i = 96;
            }
        }
    }
    if(EndDirectory == -1)
    {
        for(int i = 95; i >= 0; i--)
        {
            if(Path[i] != '\0')
            {
                EndDirectory = i + 1;
                i = -1;
            }
        }
    }
    char Directory[16];
    memset(Directory,'\0',16);
    int j = 0;
    for(int i = 1; i < EndDirectory; i++)
    {
        Directory[j] = Path[i];
        j++;
    }
    if(FatherDirectory == 1)
    {
        tvd TVD;
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        char FileContent[256];
        memset(FileContent,'\0',256);
        int ActualNumber = (*NodeNumber);
        sprintf(FileContent, "struct%d[shape=record,label=\"{%s|{<f0>|<f1>|<f2>|<f3>|<f4>|<f5>|<f6>|<f7>}}\"];\n",ActualNumber,TVD.tvd_directory_name);
        fputs(FileContent, File);
        int DirectoryFounded = -1;
        int SearchingDirectory = 1;
        do{
            for(int i = 0; i < 6; i++)
            {
                if(TVD.tvd_p_subdirectorys[i] != -1)
                {
                    tvd TemporalTVD;
                    initializeTVD(&TemporalTVD);
                    fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                    fread(&TemporalTVD,sizeof(tvd),1,Disk);
                    int NameMatchCounter = 0;
                    for(int j = 0; j < 16; j++)
                    {
                        if(Directory[j] == TemporalTVD.tvd_directory_name[j])
                            NameMatchCounter++;
                    }
                    if(NameMatchCounter == 16)
                    {
                        memset(FileContent,'\0',256);
                        (*NodeNumber) = (*NodeNumber) + 1;
                        sprintf(FileContent, "struct%d:f%d -> struct%d;\n",ActualNumber,i,(*NodeNumber));
                        fputs(FileContent, File);
                        DirectoryFounded = TVD.tvd_p_subdirectorys[i];
                    }
                }
            }
            if(DirectoryFounded == -1)
            {
                if(TVD.tvd_p_tvd != -1)
                {
                    VirtualTreePointer = TVD.tvd_p_tvd;
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                }
                else
                {
                    SearchingDirectory = 0;
                }
            }
            else
            {
                SearchingDirectory = 0;
            }
        }while(SearchingDirectory);
        if(DirectoryFounded != -1)
        {
            char NewPath[96];
            memset(NewPath,'\0',96);
            int j = 0;
            for(int i = EndDirectory; i < 96; i++)
            {
                NewPath[j] = Path[i];
                j++;
            }
            writeTreeFile(NewPath,DirectoryFounded,&(*SB),&(*Disk),&(*File),&(*NodeNumber));
        }
    }
    else
    {
        tvd TVD;
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        char FileContent[256];
        memset(FileContent,'\0',256);
        int ActualNumber = (*NodeNumber);
        sprintf(FileContent, "struct%d[shape=record,label=\"{%s|{<f0>|<f1>|<f2>|<f3>|<f4>|<f5>|<f6>|<f7>}}\"];\n",ActualNumber,TVD.tvd_directory_name);
        fputs(FileContent, File);
        int TotalFiles = 1;
        printf(">> Archivos disponibles:\n");
        if(TVD.tvd_p_detail_directory != -1)
        {
            dd DD;
            initializeDD(&DD);
            fseek(Disk,TVD.tvd_p_detail_directory,SEEK_SET);
            fread(&DD,sizeof(dd),1,Disk);
            int ShowingDetailFiles = 1;
            do
            {
                for(int i = 0; i < 5; i++)
                {
                    if(DD.files[i].dd_file_name[0] != '\0')
                    {
                        printf("%d. %s\n",TotalFiles,DD.files[i].dd_file_name);
                        TotalFiles++;
                    }
                }
                if(DD.dd_p_dd != -1)
                {
                    fseek(Disk,DD.dd_p_dd,SEEK_SET);
                    fread(&DD,sizeof(dd),1,Disk);
                }
                else
                {
                    ShowingDetailFiles = 0;
                }
            }while(ShowingDetailFiles);
        }
        int FileToGraph;
        scanf("%d",&FileToGraph);
        TotalFiles = 1;
        printf(">> Seleccione el directorio a graficar.\n");
        if(TVD.tvd_p_detail_directory != -1)
        {
            dd DD;
            initializeDD(&DD);
            fseek(Disk,TVD.tvd_p_detail_directory,SEEK_SET);
            fread(&DD,sizeof(dd),1,Disk);
            int ShowingDetailFiles = 1;
            do
            {
                for(int i = 0; i < 5; i++)
                {
                    if(DD.files[i].dd_file_name[0] != '\0')
                    {
                        if(FileToGraph == TotalFiles)
                        {
                            ShowingDetailFiles = 0;
                            (*NodeNumber) = (*NodeNumber) + 1;
                            sprintf(FileContent, "struct%d:f%d -> struct%d;\n",ActualNumber,6,(*NodeNumber));
                            fputs(FileContent, File);
                            memset(FileContent,'\0',256);
                            sprintf(FileContent, "struct%d[shape=record,label=\"{<f0>%s|<f1>|<f2>|<f3>|<f4>|<f5>}\"];\n",
                                    (*NodeNumber),DD.files[i].dd_file_name);
                            fputs(FileContent, File);
                            memset(FileContent,'\0',256);
                            int TemporalNodeNumber = (*NodeNumber) + 1;
                            sprintf(FileContent, "struct%d:f%d -> struct%d;\n",(*NodeNumber),0,TemporalNodeNumber);
                            fputs(FileContent, File);
                            int WritingInodes = 1;
                            int InodePointer = DD.files[i].dd_file_p_inode;
                            int InodeNodeNumber = TemporalNodeNumber;
                            do
                            {
                                inode INODE;
                                fseek(Disk,InodePointer,SEEK_SET);
                                fread(&INODE,sizeof(inode),1,Disk);
                                memset(FileContent,'\0',256);
                                sprintf(FileContent, "struct%d[shape=record,label=\"{<f0>I: %d|<f1>B: %d|<f2> S: %d|<f3>P: %d|<f4>|<f5>}\"];\n",InodeNodeNumber
                                        ,INODE.i_count,INODE.i_block_count,INODE.i_file_size,INODE.i_proprietary);
                                fputs(FileContent, File);
                                for(int j = 0; j < 4; j++)
                                {
                                    if(INODE.i_p_block[j] != -1)
                                    {
                                        TemporalNodeNumber = TemporalNodeNumber + 1;
                                        memset(FileContent,'\0',256);
                                        sprintf(FileContent, "struct%d:f4 -> struct%d;\n",InodeNodeNumber,TemporalNodeNumber);
                                        fputs(FileContent, File);
                                        block BLOCK;
                                        fseek(Disk,INODE.i_p_block[j],SEEK_SET);
                                        fread(&BLOCK,sizeof(block),1,Disk);
                                        memset(FileContent,'\0',256);
                                        sprintf(FileContent, "struct%d[shape=record,label=\"{ %s }\"];\n",TemporalNodeNumber,BLOCK.block_data);
                                        fputs(FileContent, File);
                                    }
                                }
                                if(INODE.i_p_i != -1)
                                {
                                    TemporalNodeNumber = TemporalNodeNumber + 1;
                                    memset(FileContent,'\0',256);
                                    sprintf(FileContent, "struct%d:f5 -> struct%d;\n",InodeNodeNumber,TemporalNodeNumber);
                                    fputs(FileContent, File);
                                    InodePointer = INODE.i_p_i;
                                    InodeNodeNumber = TemporalNodeNumber;
                                }
                                else
                                {
                                    WritingInodes = 0;
                                }
                            }while(WritingInodes);
                        }
                        TotalFiles++;
                    }
                }
                if(DD.dd_p_dd != -1)
                {
                    fseek(Disk,DD.dd_p_dd,SEEK_SET);
                    fread(&DD,sizeof(dd),1,Disk);
                }
                else
                {
                    ShowingDetailFiles = 0;
                }
            }while(ShowingDetailFiles);
        }
    }
}

void writeTreeDirectory(char *Path, int VirtualTreePointer, sb *SB, FILE *Disk, FILE *File)
{
    int EndDirectory = -1;
    int SlashCounter = 0;
    int FatherDirectory = 0;
    for(int i = 0; i < 96; i++)
    {
        if(Path[i] == '/')
        {
            SlashCounter++;
            if(SlashCounter == 2)
            {
                FatherDirectory = 1;
                EndDirectory = i;
                i = 96;
            }
        }
    }
    if(EndDirectory == -1)
    {
        for(int i = 95; i >= 0; i--)
        {
            if(Path[i] != '\0')
            {
                EndDirectory = i + 1;
                i = -1;
            }
        }
    }
    char Directory[16];
    memset(Directory,'\0',16);
    int j = 0;
    for(int i = 1; i < EndDirectory; i++)
    {
        Directory[j] = Path[i];
        j++;
    }
    if(FatherDirectory == 1)
    {
        tvd TVD;
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        int DirectoryFounded = -1;
        int SearchingDirectory = 1;
        do{
            for(int i = 0; i < 6; i++)
            {
                if(TVD.tvd_p_subdirectorys[i] != -1)
                {
                    tvd TemporalTVD;
                    initializeTVD(&TemporalTVD);
                    fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                    fread(&TemporalTVD,sizeof(tvd),1,Disk);
                    int NameMatchCounter = 0;
                    for(int j = 0; j < 16; j++)
                    {
                        if(Directory[j] == TemporalTVD.tvd_directory_name[j])
                            NameMatchCounter++;
                    }
                    if(NameMatchCounter == 16)
                    {
                        DirectoryFounded = TVD.tvd_p_subdirectorys[i];
                    }
                }
            }
            if(DirectoryFounded == -1)
            {
                if(TVD.tvd_p_tvd != -1)
                {
                    VirtualTreePointer = TVD.tvd_p_tvd;
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                }
                else
                {
                    SearchingDirectory = 0;
                }
            }
            else
            {
                SearchingDirectory = 0;
            }
        }while(SearchingDirectory);
        if(DirectoryFounded != -1)
        {
            char NewPath[96];
            memset(NewPath,'\0',96);
            int j = 0;
            for(int i = EndDirectory; i < 96; i++)
            {
                NewPath[j] = Path[i];
                j++;
            }
            writeTreeDirectory(NewPath,DirectoryFounded,&(*SB),&(*Disk),&(*File));
        }
    }
    else
    {
        tvd TVD;
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        int TotalDirectorys = 1;
        int ShowingDirectorys = 1;
        printf(">> Directorios disponibles:\n");
        do
        {
            for(int i = 0; i < 6; i++)
            {
                if(TVD.tvd_p_subdirectorys[i] != -1)
                {
                    tvd TemporalTVD;
                    initializeTVD(&TemporalTVD);
                    fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                    fread(&TemporalTVD,sizeof(tvd),1,Disk);
                    printf("%d. %s\n",TotalDirectorys,TemporalTVD.tvd_directory_name);
                    TotalDirectorys++;
                }
            }
            if(TVD.tvd_p_tvd != -1)
            {
                fseek(Disk,TVD.tvd_p_tvd,SEEK_SET);
                fread(&TVD,sizeof(tvd),1,Disk);
            }
            else
            {
                ShowingDirectorys = 0;
            }
        }
        while(ShowingDirectorys);
        int DirectoryToGraph;
        scanf("%d",&DirectoryToGraph);
        TotalDirectorys = 1;
        int SearchingDirectory = 1;
        printf(">> Seleccione el directorio a graficar.\n");
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        do
        {
            for(int i = 0; i < 6; i++)
            {
                if(TVD.tvd_p_subdirectorys[i] != -1)
                {
                    if(DirectoryToGraph == TotalDirectorys)
                    {
                        initializeTVD(&TVD);
                        fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                        fread(&TVD,sizeof(tvd),1,Disk);
                        SearchingDirectory = 0;
                        int ActualNumber = 0;
                        int NodeNumber = 0;
                        char FileContent[256];
                        memset(FileContent,'\0',256);
                        sprintf(FileContent, "struct%d[shape=record,label=\"{%s|{<f0>|<f1>|<f2>|<f3>|<f4>|<f5>|<f6>|<f7>}}\"];\n",ActualNumber,TVD.tvd_directory_name);
                        fputs(FileContent, File);
                        if(TVD.tvd_p_detail_directory != -1)
                        {
                            memset(FileContent,'\0',256);
                            NodeNumber = NodeNumber + 1;
                            sprintf(FileContent, "struct%d:f7 -> struct%d;\n",ActualNumber,NodeNumber);
                            fputs(FileContent, File);
                            int KeepWritingDetails = 1;
                            int DetailDirectoryPointer = TVD.tvd_p_detail_directory;
                            int DetailNodeNumber = NodeNumber;
                            do
                            {
                                dd DD;
                                fseek(Disk,DetailDirectoryPointer,SEEK_SET);
                                fread(&DD,sizeof(dd),1,Disk);
                                memset(FileContent,'\0',256);
                                sprintf(FileContent, "struct%d[shape=record,label=\"{<f0>%s|<f1>%s|<f2>%s|<f3>%s|<f4>%s|<f5>}\"];\n",DetailNodeNumber
                                        ,DD.files[0].dd_file_name,DD.files[1].dd_file_name,DD.files[2].dd_file_name,DD.files[3].dd_file_name,
                                        DD.files[4].dd_file_name);
                                fputs(FileContent, File);
                                for(int i = 0; i < 5; i++)
                                {
                                    if(DD.files[i].dd_file_name[0] != '\0')
                                    {
                                        memset(FileContent,'\0',256);
                                        NodeNumber = NodeNumber + 1;
                                        sprintf(FileContent, "struct%d:f%d -> struct%d;\n",DetailNodeNumber,i,NodeNumber);
                                        fputs(FileContent, File);
                                        int WritingInodes = 1;
                                        int InodePointer = DD.files[i].dd_file_p_inode;
                                        int InodeNodeNumber = NodeNumber;
                                        do
                                        {
                                            inode INODE;
                                            fseek(Disk,InodePointer,SEEK_SET);
                                            fread(&INODE,sizeof(inode),1,Disk);
                                            memset(FileContent,'\0',256);
                                            sprintf(FileContent, "struct%d[shape=record,label=\"{<f0>I: %d|<f1>B: %d|<f2> S: %d|<f3>P: %d|<f4>|<f5>}\"];\n",InodeNodeNumber
                                                    ,INODE.i_count,INODE.i_block_count,INODE.i_file_size,INODE.i_proprietary);
                                            fputs(FileContent, File);
                                            for(int j = 0; j < 4; j++)
                                            {
                                                if(INODE.i_p_block[j] != -1)
                                                {
                                                    NodeNumber = NodeNumber + 1;
                                                    memset(FileContent,'\0',256);
                                                    sprintf(FileContent, "struct%d:f4 -> struct%d;\n",InodeNodeNumber,NodeNumber);
                                                    fputs(FileContent, File);
                                                    block BLOCK;
                                                    fseek(Disk,INODE.i_p_block[j],SEEK_SET);
                                                    fread(&BLOCK,sizeof(block),1,Disk);
                                                    memset(FileContent,'\0',256);
                                                    sprintf(FileContent, "struct%d[shape=record,label=\"{ %s }\"];\n",NodeNumber,BLOCK.block_data);
                                                    fputs(FileContent, File);
                                                }
                                            }
                                            if(INODE.i_p_i != -1)
                                            {
                                                NodeNumber = NodeNumber + 1;
                                                memset(FileContent,'\0',256);
                                                sprintf(FileContent, "struct%d:f5 -> struct%d;\n",InodeNodeNumber,NodeNumber);
                                                fputs(FileContent, File);
                                                InodePointer = INODE.i_p_i;
                                                InodeNodeNumber = NodeNumber;
                                            }
                                            else
                                            {
                                                WritingInodes = 0;
                                            }
                                        }while(WritingInodes);
                                    }
                                }
                                if(DD.dd_p_dd != -1)
                                {
                                    NodeNumber = NodeNumber + 1;
                                    memset(FileContent,'\0',256);
                                    sprintf(FileContent, "struct%d:f5 -> struct%d;\n",DetailNodeNumber,NodeNumber);
                                    fputs(FileContent, File);
                                    DetailDirectoryPointer = DD.dd_p_dd;
                                    DetailNodeNumber = NodeNumber;
                                }
                                else
                                {
                                    KeepWritingDetails = 0;
                                }
                            }while(KeepWritingDetails);
                        }
                    }
                    else
                    {
                        tvd TemporalTVD;
                        initializeTVD(&TemporalTVD);
                        fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                        fread(&TemporalTVD,sizeof(tvd),1,Disk);
                        printf("%d. %s\n",TotalDirectorys,TemporalTVD.tvd_directory_name);
                    }
                    TotalDirectorys++;
                }
            }
            if(TVD.tvd_p_tvd != -1)
            {
                fseek(Disk,TVD.tvd_p_tvd,SEEK_SET);
                fread(&TVD,sizeof(tvd),1,Disk);
            }
            else
            {
                SearchingDirectory = 0;
            }
        }
        while(SearchingDirectory);
    }
}

void writeCompleteReport(int VirtualTreePointer, int *NodeNumber, FILE *Disk, FILE *File)
{
    int ActualNumber = (*NodeNumber);
    tvd TVD;
    fseek(Disk,VirtualTreePointer,SEEK_SET);
    fread(&TVD,sizeof(tvd),1,Disk);
    char FileContent[256];
    memset(FileContent,'\0',256);
    sprintf(FileContent, "struct%d[shape=record,label=\"{%s|{<f0>|<f1>|<f2>|<f3>|<f4>|<f5>|<f6>|<f7>}}\"];\n",ActualNumber,TVD.tvd_directory_name);
    fputs(FileContent, File);
    for(int i = 0; i < 6; i++)
    {
        if(TVD.tvd_p_subdirectorys[i] != -1)
        {
            memset(FileContent,'\0',256);
            (*NodeNumber) = (*NodeNumber) + 1;
            sprintf(FileContent, "struct%d:f%d -> struct%d;\n",ActualNumber,i,(*NodeNumber));
            fputs(FileContent, File);
            writeCompleteReport(TVD.tvd_p_subdirectorys[i],&(*NodeNumber),&(*Disk),&(*File));
        }
    }
    if(TVD.tvd_p_tvd != -1)
    {
        memset(FileContent,'\0',256);
        (*NodeNumber) = (*NodeNumber) + 1;
        sprintf(FileContent, "struct%d:f6 -> struct%d;\n",ActualNumber,(*NodeNumber));
        fputs(FileContent, File);
        writeCompleteReport(TVD.tvd_p_tvd,&(*NodeNumber),&(*Disk),&(*File));
    }
    if(TVD.tvd_p_detail_directory != -1)
    {
        memset(FileContent,'\0',256);
        (*NodeNumber) = (*NodeNumber) + 1;
        sprintf(FileContent, "struct%d:f7 -> struct%d;\n",ActualNumber,(*NodeNumber));
        fputs(FileContent, File);
        int KeepWritingDetails = 1;
        int DetailDirectoryPointer = TVD.tvd_p_detail_directory;
        int DetailNodeNumber = (*NodeNumber);
        do
        {
            dd DD;
            fseek(Disk,DetailDirectoryPointer,SEEK_SET);
            fread(&DD,sizeof(dd),1,Disk);
            memset(FileContent,'\0',256);
            sprintf(FileContent, "struct%d[shape=record,label=\"{<f0>%s|<f1>%s|<f2>%s|<f3>%s|<f4>%s|<f5>}\"];\n",DetailNodeNumber
                    ,DD.files[0].dd_file_name,DD.files[1].dd_file_name,DD.files[2].dd_file_name,DD.files[3].dd_file_name,
                    DD.files[4].dd_file_name);
            fputs(FileContent, File);
            for(int i = 0; i < 5; i++)
            {
                if(DD.files[i].dd_file_name[0] != '\0')
                {
                    memset(FileContent,'\0',256);
                    (*NodeNumber) = (*NodeNumber) + 1;
                    sprintf(FileContent, "struct%d:f%d -> struct%d;\n",DetailNodeNumber,i,(*NodeNumber));
                    fputs(FileContent, File);
                    int WritingInodes = 1;
                    int InodePointer = DD.files[i].dd_file_p_inode;
                    int InodeNodeNumber = (*NodeNumber);
                    do
                    {
                        inode INODE;
                        fseek(Disk,InodePointer,SEEK_SET);
                        fread(&INODE,sizeof(inode),1,Disk);
                        memset(FileContent,'\0',256);
                        sprintf(FileContent, "struct%d[shape=record,label=\"{<f0>I: %d|<f1>B: %d|<f2> S: %d|<f3>P: %d|<f4>|<f5>}\"];\n",InodeNodeNumber
                                ,INODE.i_count,INODE.i_block_count,INODE.i_file_size,INODE.i_proprietary);
                        fputs(FileContent, File);
                        for(int j = 0; j < 4; j++)
                        {
                            if(INODE.i_p_block[j] != -1)
                            {
                                (*NodeNumber) = (*NodeNumber) + 1;
                                memset(FileContent,'\0',256);
                                sprintf(FileContent, "struct%d:f4 -> struct%d;\n",InodeNodeNumber,(*NodeNumber));
                                fputs(FileContent, File);
                                block BLOCK;
                                fseek(Disk,INODE.i_p_block[j],SEEK_SET);
                                fread(&BLOCK,sizeof(block),1,Disk);
                                memset(FileContent,'\0',256);
                                sprintf(FileContent, "struct%d[shape=record,label=\"{ %s }\"];\n",(*NodeNumber),BLOCK.block_data);
                                fputs(FileContent, File);
                            }
                        }
                        if(INODE.i_p_i != -1)
                        {
                            (*NodeNumber) = (*NodeNumber) + 1;
                            memset(FileContent,'\0',256);
                            sprintf(FileContent, "struct%d:f5 -> struct%d;\n",InodeNodeNumber,(*NodeNumber));
                            fputs(FileContent, File);
                            InodePointer = INODE.i_p_i;
                            InodeNodeNumber = (*NodeNumber);
                        }
                        else
                        {
                            WritingInodes = 0;
                        }
                    }while(WritingInodes);
                }
            }
            if(DD.dd_p_dd != -1)
            {
                (*NodeNumber) = (*NodeNumber) + 1;
                memset(FileContent,'\0',256);
                sprintf(FileContent, "struct%d:f5 -> struct%d;\n",DetailNodeNumber,(*NodeNumber));
                fputs(FileContent, File);
                DetailDirectoryPointer = DD.dd_p_dd;
                DetailNodeNumber = (*NodeNumber);
            }
            else
            {
                KeepWritingDetails = 0;
            }

        }while(KeepWritingDetails);
    }
}

void writeDirectoryReport(int VirtualTreePointer, int *NodeNumber, FILE *Disk, FILE *File)
{
    int ActualNumber = (*NodeNumber);
    tvd TVD;
    fseek(Disk,VirtualTreePointer,SEEK_SET);
    fread(&TVD,sizeof(tvd),1,Disk);
    char FileContent[256];
    memset(FileContent,'\0',256);
    sprintf(FileContent, "struct%d[shape=record,label=\"{%s|{<f0>|<f1>|<f2>|<f3>|<f4>|<f5>|<f6>}}\"];\n",ActualNumber,TVD.tvd_directory_name);
    fputs(FileContent, File);
    for(int i = 0; i < 6; i++)
    {
        if(TVD.tvd_p_subdirectorys[i] != -1)
        {
            memset(FileContent,'\0',256);
            (*NodeNumber) = (*NodeNumber) + 1;
            sprintf(FileContent, "struct%d:f%d -> struct%d;\n",ActualNumber,i,(*NodeNumber));
            fputs(FileContent, File);
            writeDirectoryReport(TVD.tvd_p_subdirectorys[i],&(*NodeNumber),&(*Disk),&(*File));
        }
    }
    if(TVD.tvd_p_tvd != -1)
    {
        memset(FileContent,'\0',256);
        (*NodeNumber) = (*NodeNumber) + 1;
        sprintf(FileContent, "struct%d:f6 -> struct%d;\n",ActualNumber,(*NodeNumber));
        fputs(FileContent, File);
        writeDirectoryReport(TVD.tvd_p_tvd,&(*NodeNumber),&(*Disk),&(*File));
    }
}

void runREP(rep *REP)
{
    int MountedIndex = -1;
    for(int i = 0; i < 256; i++)
    {
        if(MountedPartitions[i].active == '1' && MountedPartitions[i].letter == (*REP).id[2]
                && MountedPartitions[i].number == ((*REP).id[3] - 48))
        {
            MountedIndex = i;
            i = 256;
        }
    }
    if(MountedIndex != -1)
    {
        FILE *disk;
        if((disk = fopen(MountedPartitions[MountedIndex].path,"rb+")))
        {
            mbr MBR;
            fseek(disk,0,SEEK_SET);
            fread(&MBR,sizeof(mbr),1,disk);
            int NameMatch[4];
            NameMatch[0] = 0;
            NameMatch[1] = 0;
            NameMatch[2] = 0;
            NameMatch[3] = 0;
            for(int i = 0; i < 16; i++)
            {
                if(MBR.mbr_partition_1.part_status == '1'
                        && MBR.mbr_partition_1.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[0]++;
                if(MBR.mbr_partition_2.part_status == '1'
                        && MBR.mbr_partition_2.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[1]++;
                if(MBR.mbr_partition_3.part_status == '1'
                        && MBR.mbr_partition_3.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[2]++;
                if(MBR.mbr_partition_4.part_status == '1'
                        && MBR.mbr_partition_4.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[3]++;
            }
            int PartitionStartIndex = 0;
            int PartitionTotalSize = 0;
            if(NameMatch[0] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_1.part_start;
                PartitionTotalSize = MBR.mbr_partition_1.part_size;
            }
            if(NameMatch[1] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_2.part_start;
                PartitionTotalSize = MBR.mbr_partition_2.part_size;
            }
            if(NameMatch[2] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_3.part_start;
                PartitionTotalSize = MBR.mbr_partition_3.part_size;
            }
            if(NameMatch[3] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_4.part_start;
                PartitionTotalSize = MBR.mbr_partition_4.part_size;
            }
            if(NameMatch[0] == 16 || NameMatch[1] == 16 || NameMatch[2] == 16 || NameMatch[3] == 16)
            {
                fseek(disk,PartitionStartIndex,SEEK_SET);
                sb SB;
                initializeSB(&SB);
                fread(&SB,sizeof(sb),1,disk);
                if(SB.sb_magic_number == 201504468)
                {
                    char BMDIR[14];
                    memset(BMDIR,'\0',14);
                    strcpy(BMDIR,"bm_arbdir");
                    if(strcmp(BMDIR,(*REP).name) == 0)
                    {
                        printf(">> Bitmap Arbol Virtual de Diretorio.\n");
                        int NewLine = 0;
                        int NewSpace = 0;
                        for(int i = SB.sb_p_tree_directory_bitmap; i < SB.sb_p_tree_directory; i++)
                        {
                            char Bit;
                            fseek(disk,i,SEEK_SET);
                            fread(&Bit,sizeof(char),1,disk);
                            if(NewLine < 19)
                            {
                                printf("[%c]",Bit);
                                NewLine++;
                                if(NewSpace == 4)
                                {
                                    printf(" ");
                                    NewSpace = 0;
                                }
                                else
                                {
                                    NewSpace++;
                                }
                            }
                            else
                            {
                                printf("[%c]\n",Bit);
                                NewLine = 0;
                                NewSpace = 0;
                            }
                        }
                    }
                    char BMDET[14];
                    memset(BMDET,'\0',14);
                    strcpy(BMDET,"bm_detdir");
                    if(strcmp(BMDET,(*REP).name) == 0)
                    {
                        printf(">> Bitmap Detalle de Directorio.\n");
                        int NewLine = 0;
                        int NewSpace = 0;
                        for(int i = SB.sb_p_detail_directory_bitmap; i < SB.sb_p_detail_directory; i++)
                        {
                            char Bit;
                            fseek(disk,i,SEEK_SET);
                            fread(&Bit,sizeof(char),1,disk);
                            if(NewLine < 19)
                            {
                                printf("[%c]",Bit);
                                NewLine++;
                                if(NewSpace == 4)
                                {
                                    printf(" ");
                                    NewSpace = 0;
                                }
                                else
                                {
                                    NewSpace++;
                                }
                            }
                            else
                            {
                                printf("[%c]\n",Bit);
                                NewLine = 0;
                                NewSpace = 0;
                            }
                        }
                    }
                    char BMINODE[14];
                    memset(BMINODE,'\0',14);
                    strcpy(BMINODE,"bm_inode");
                    if(strcmp(BMINODE,(*REP).name) == 0)
                    {
                        printf(">> Bitmap Inodos.\n");
                        int NewLine = 0;
                        int NewSpace = 0;
                        for(int i = SB.sb_p_inode_bitmap; i < SB.sb_p_inode; i++)
                        {
                            char Bit;
                            fseek(disk,i,SEEK_SET);
                            fread(&Bit,sizeof(char),1,disk);
                            if(NewLine < 19)
                            {
                                printf("[%c]",Bit);
                                NewLine++;
                                if(NewSpace == 4)
                                {
                                    printf(" ");
                                    NewSpace = 0;
                                }
                                else
                                {
                                    NewSpace++;
                                }
                            }
                            else
                            {
                                printf("[%c]\n",Bit);
                                NewLine = 0;
                                NewSpace = 0;
                            }
                        }
                    }
                    char BMBLOCK[14];
                    memset(BMBLOCK,'\0',14);
                    strcpy(BMBLOCK,"bm_block");
                    if(strcmp(BMBLOCK,(*REP).name) == 0)
                    {
                        printf(">> Bitmap Bloques\n");
                        int NewLine = 0;
                        int NewSpace = 0;
                        for(int i = SB.sb_p_block_bitmap; i < SB.sb_p_block; i++)
                        {
                            char Bit;
                            fseek(disk,i,SEEK_SET);
                            fread(&Bit,sizeof(char),1,disk);
                            if(NewLine < 19)
                            {
                                printf("[%c]",Bit);
                                NewLine++;
                                if(NewSpace == 4)
                                {
                                    printf(" ");
                                    NewSpace = 0;
                                }
                                else
                                {
                                    NewSpace++;
                                }
                            }
                            else
                            {
                                printf("[%c]\n",Bit);
                                NewLine = 0;
                                NewSpace = 0;
                            }
                        }
                    }
                    char BMDIRECTORIO[14];
                    memset(BMDIRECTORIO,'\0',14);
                    strcpy(BMDIRECTORIO,"directorio");
                    if(strcmp(BMDIRECTORIO,(*REP).name) == 0)
                    {
                        int PathDirectoryLength = 0;
                        for(int i = 95; i >= 0; i--)
                        {
                            if((*REP).path[i] == '/')
                            {
                                PathDirectoryLength = i + 1;
                                i = -1;
                            }
                        }
                        char DirectoryPath[PathDirectoryLength];
                        for(int i = 0; i < PathDirectoryLength; i++)
                        {
                            DirectoryPath[i] = (*REP).path[i];
                        }
                        FILE *Dot;
                        Dot = fopen("source.dot", "w+");
                        fputs("digraph structs {\nnode [shape=record];\n", Dot);
                        int NodeNumber = 0;
                        writeDirectoryReport(SB.sb_p_tree_directory,&NodeNumber,&(*disk),&(*Dot));
                        fputs("}", Dot);
                        fclose(Dot);
                        createDirectory(DirectoryPath,PathDirectoryLength);
                        char GraphCompileCommand[128];
                        memset(GraphCompileCommand,'\0',128);
                        sprintf(GraphCompileCommand, "dot -Tps source.dot -o %s", (*REP).path);
                        system(GraphCompileCommand);
                        char OpenFileCommand[64];
                        memset(OpenFileCommand,'\0',64);
                        sprintf(OpenFileCommand, "evince  %s", (*REP).path);
                        system(OpenFileCommand);
                    }
                    char TREECOMPLETE[14];
                    memset(TREECOMPLETE,'\0',14);
                    strcpy(TREECOMPLETE,"tree_complete");
                    if(strcmp(TREECOMPLETE,(*REP).name) == 0)
                    {
                        int PathDirectoryLength = 0;
                        for(int i = 95; i >= 0; i--)
                        {
                            if((*REP).path[i] == '/')
                            {
                                PathDirectoryLength = i + 1;
                                i = -1;
                            }
                        }
                        char DirectoryPath[PathDirectoryLength];
                        for(int i = 0; i < PathDirectoryLength; i++)
                        {
                            DirectoryPath[i] = (*REP).path[i];
                        }
                        FILE *Dot;
                        Dot = fopen("source.dot", "w+");
                        fputs("digraph structs {\nnode [shape=record];\n", Dot);
                        int NodeNumber = 0;
                        writeCompleteReport(SB.sb_p_tree_directory,&NodeNumber,&(*disk),&(*Dot));
                        fputs("}", Dot);
                        fclose(Dot);
                        createDirectory(DirectoryPath,PathDirectoryLength);
                        char GraphCompileCommand[128];
                        memset(GraphCompileCommand,'\0',128);
                        sprintf(GraphCompileCommand, "dot -Tps source.dot -o %s", (*REP).path);
                        system(GraphCompileCommand);
                        char OpenFileCommand[64];
                        memset(OpenFileCommand,'\0',64);
                        sprintf(OpenFileCommand, "evince  %s", (*REP).path);
                        system(OpenFileCommand);
                    }
                    char TREEDIRECTORY[14];
                    memset(TREEDIRECTORY,'\0',14);
                    strcpy(TREEDIRECTORY,"tree_direct");
                    if(strcmp(TREEDIRECTORY,(*REP).name) == 0)
                    {
                        int PathDirectoryLength = 0;
                        for(int i = 95; i >= 0; i--)
                        {
                            if((*REP).path[i] == '/')
                            {
                                PathDirectoryLength = i + 1;
                                i = -1;
                            }
                        }
                        char DirectoryPath[PathDirectoryLength];
                        for(int i = 0; i < PathDirectoryLength; i++)
                        {
                            DirectoryPath[i] = (*REP).path[i];
                        }
                        FILE *Dot;
                        Dot = fopen("source.dot", "w+");
                        fputs("digraph structs {\nnode [shape=record];\n", Dot);
                        writeTreeDirectory((*REP).ruta,SB.sb_p_tree_directory,&SB,&(*disk),&(*Dot));
                        fputs("}", Dot);
                        fclose(Dot);
                        createDirectory(DirectoryPath,PathDirectoryLength);
                        char GraphCompileCommand[128];
                        memset(GraphCompileCommand,'\0',128);
                        sprintf(GraphCompileCommand, "dot -Tps source.dot -o %s", (*REP).path);
                        system(GraphCompileCommand);
                        char OpenFileCommand[64];
                        memset(OpenFileCommand,'\0',64);
                        sprintf(OpenFileCommand, "evince  %s", (*REP).path);
                        system(OpenFileCommand);
                    }
                    char TREEFILE[14];
                    memset(TREEFILE,'\0',14);
                    strcpy(TREEFILE,"tree_file");
                    if(strcmp(TREEFILE,(*REP).name) == 0)
                    {
                        int PathDirectoryLength = 0;
                        for(int i = 95; i >= 0; i--)
                        {
                            if((*REP).path[i] == '/')
                            {
                                PathDirectoryLength = i + 1;
                                i = -1;
                            }
                        }
                        char DirectoryPath[PathDirectoryLength];
                        for(int i = 0; i < PathDirectoryLength; i++)
                        {
                            DirectoryPath[i] = (*REP).path[i];
                        }
                        FILE *Dot;
                        Dot = fopen("source.dot", "w+");
                        fputs("digraph structs {\nnode [shape=record];\n", Dot);
                        int NodeNumber = 0;
                        writeTreeFile((*REP).ruta,SB.sb_p_tree_directory,&SB,&(*disk),&(*Dot),&NodeNumber);
                        fputs("}", Dot);
                        fclose(Dot);
                        createDirectory(DirectoryPath,PathDirectoryLength);
                        char GraphCompileCommand[128];
                        memset(GraphCompileCommand,'\0',128);
                        sprintf(GraphCompileCommand, "dot -Tps source.dot -o %s", (*REP).path);
                        system(GraphCompileCommand);
                        char OpenFileCommand[64];
                        memset(OpenFileCommand,'\0',64);
                        sprintf(OpenFileCommand, "evince  %s", (*REP).path);
                        system(OpenFileCommand);
                    }
                    char DISKREPORT[14];
                    memset(DISKREPORT,'\0',14);
                    strcpy(DISKREPORT,"disk");
                    if(strcmp(DISKREPORT,(*REP).name) == 0)
                    {
                        int PathDirectoryLength = 0;
                        for(int i = 95; i >= 0; i--)
                        {
                            if((*REP).path[i] == '/')
                            {
                                PathDirectoryLength = i + 1;
                                i = -1;
                            }
                        }
                        char DirectoryPath[PathDirectoryLength];
                        for(int i = 0; i < PathDirectoryLength; i++)
                        {
                            DirectoryPath[i] = (*REP).path[i];
                        }
                        FILE *Dot;
                        Dot = fopen("source.dot", "w+");
                        fputs("digraph structs {\nnode [shape=record];\n", Dot);
                        char Content[512];
                        memset(Content,'\0',512);
                        time_t Time = MBR.mbr_creation_date;
                        struct tm *tlocal = localtime(&Time);
                        char TemporalTime[24];
                        strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                        strcpy(SB.sb_date_last_montage,TemporalTime);
                        char Partition1Name[16];
                        char Partition2Name[16];
                        char Partition3Name[16];
                        char Partition4Name[16];
                        memset(Partition1Name,'0',16);
                        memset(Partition2Name,'0',16);
                        memset(Partition3Name,'0',16);
                        memset(Partition4Name,'0',16);
                        Partition1Name[15] = '\0';
                        Partition2Name[15] = '\0';
                        Partition3Name[15] = '\0';
                        Partition4Name[15] = '\0';
                        char MBRPartitionType[4];
                        memset(MBRPartitionType,'0',4);
                        int MBRPartitionStart[4];
                        MBRPartitionStart[0] = -1;
                        MBRPartitionStart[1] = -1;
                        MBRPartitionStart[2] = -1;
                        MBRPartitionStart[3] = -1;
                        int MBRPartitionSize[4];
                        MBRPartitionSize[0] = -1;
                        MBRPartitionSize[1] = -1;
                        MBRPartitionSize[2] = -1;
                        MBRPartitionSize[3] = -1;
                        if(MBR.mbr_partition_1.part_status == '1')
                        {
                            strcpy(Partition1Name,MBR.mbr_partition_1.part_name);
                            MBRPartitionStart[0] = MBR.mbr_partition_1.part_start;
                            MBRPartitionSize[0] = MBR.mbr_partition_1.part_size;
                            MBRPartitionType[0] = MBR.mbr_partition_1.part_type;
                        }
                        if(MBR.mbr_partition_2.part_status == '1')
                        {
                            strcpy(Partition2Name,MBR.mbr_partition_2.part_name);
                            MBRPartitionStart[1] = MBR.mbr_partition_2.part_start;
                            MBRPartitionSize[1] = MBR.mbr_partition_2.part_size;
                            MBRPartitionType[1] = MBR.mbr_partition_2.part_type;
                        }
                        if(MBR.mbr_partition_3.part_status == '1')
                        {
                            strcpy(Partition3Name,MBR.mbr_partition_3.part_name);
                            MBRPartitionStart[2] = MBR.mbr_partition_3.part_start;
                            MBRPartitionSize[2] = MBR.mbr_partition_3.part_size;
                            MBRPartitionType[2] = MBR.mbr_partition_3.part_type;
                        }
                        if(MBR.mbr_partition_4.part_status == '1')
                        {
                            strcpy(Partition4Name,MBR.mbr_partition_4.part_name);
                            MBRPartitionStart[3] = MBR.mbr_partition_4.part_start;
                            MBRPartitionSize[3] = MBR.mbr_partition_4.part_size;
                            MBRPartitionType[3] = MBR.mbr_partition_4.part_type;
                        }
                        sprintf(Content, "struct0[shape=record,label=\"{{{MBR|Size: %d|Signature: %d|Creation: %s}|{%s|%c|{%d|%d}}|{%s|%c|{%d|%d}}|{%s|%c|{%d|%d}}|{%s|%c|{%d|%d}}}}\"];",
                                MBR.mbr_size,MBR.mbr_disk_signature,TemporalTime,
                                Partition1Name,MBRPartitionType[0],MBRPartitionStart[0],MBRPartitionSize[0],
                                Partition2Name,MBRPartitionType[1],MBRPartitionStart[1],MBRPartitionSize[1],
                                Partition3Name,MBRPartitionType[2],MBRPartitionStart[2],MBRPartitionSize[2],
                                Partition4Name,MBRPartitionType[3],MBRPartitionStart[3],MBRPartitionSize[3]);
                        fputs(Content,Dot);
                        fputs("}", Dot);
                        fclose(Dot);
                        createDirectory(DirectoryPath,PathDirectoryLength);
                        char GraphCompileCommand[128];
                        memset(GraphCompileCommand,'\0',128);
                        sprintf(GraphCompileCommand, "dot -Tps source.dot -o %s", (*REP).path);
                        system(GraphCompileCommand);
                        char OpenFileCommand[64];
                        memset(OpenFileCommand,'\0',64);
                        sprintf(OpenFileCommand, "evince  %s", (*REP).path);
                        system(OpenFileCommand);
                    }
                    char SBREPORT[14];
                    memset(SBREPORT,'\0',14);
                    strcpy(SBREPORT,"sb");
                    if(strcmp(SBREPORT,(*REP).name) == 0)
                    {
                        int PathDirectoryLength = 0;
                        for(int i = 95; i >= 0; i--)
                        {
                            if((*REP).path[i] == '/')
                            {
                                PathDirectoryLength = i + 1;
                                i = -1;
                            }
                        }
                        char DirectoryPath[PathDirectoryLength];
                        for(int i = 0; i < PathDirectoryLength; i++)
                        {
                            DirectoryPath[i] = (*REP).path[i];
                        }
                        FILE *Dot;
                        Dot = fopen("source.dot", "w+");
                        fputs("digraph structs {\nnode [shape=record];\n", Dot);
                        char Content[2048];
                        memset(Content,'\0',2048);
                        time_t Time = MBR.mbr_creation_date;
                        struct tm *tlocal = localtime(&Time);
                        char TemporalTime[24];
                        strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                        strcpy(SB.sb_date_last_montage,TemporalTime);
                        sprintf(Content, "struct0[shape=record,margin=0.3,label=\"{{disk_name |%s}|{virtual_tree_count|%d}|"
                                         "{detail_directory_count|%d}|{inode_count|%d}|{block_count|%d}|{virtual_tree_free|"
                                         "%d}|{detail_directory_free|%d}|{inode_free|%d}|{block_free|%d}|{date_creation|%s}|"
                                         "{date_last_montage|%s}|{montage_count|%d}|{p_tree_directory_bitmap|%d}|"
                                         "{p_tree_directory|%d}|{p_detail_directory_bitmap|%d}|{p_detail_directory|%d}|"
                                         "{p_inode_bitmap|%d}|{p_inode|%d}|{p_block_bitmap|%d}|{p_block|%d}|{p_log|%d}|"
                                         "{p_backup|%d}|{s_tree_directory|%d}|{s_detail_directory|%d}|{s_inode|%d}|"
                                         "{s_block|%d}|{ffb_tree_directory|%d}|{ffb_detail_directory|%d}|{ffb_inode|%d}"
                                         "|{ffb_block|%d}|{magic_number|%d}}\"];",
                                SB.sb_disk_name,SB.sb_virtual_tree_count,SB.sb_detail_directory_count,SB.sb_inode_count,SB.sb_block_count
                                ,SB.sb_virtual_tree_free,SB.sb_detail_directory_free,SB.sb_inode_free,SB.sb_block_free,SB.sb_date_creation,
                                SB.sb_date_last_montage,SB.sb_montage_count,SB.sb_p_tree_directory_bitmap,SB.sb_p_tree_directory,
                                SB.sb_p_detail_directory_bitmap,SB.sb_p_detail_directory,SB.sb_p_inode_bitmap,SB.sb_p_inode,
                                SB.sb_p_block_bitmap,SB.sb_p_block,SB.sb_p_log,SB.sb_p_backup,SB.sb_s_tree_directory,SB.sb_s_detail_directory
                                ,SB.sb_s_inode,SB.sb_s_block,SB.sb_ffb_tree_directory,SB.sb_ffb_detail_directory,SB.sb_ffb_inode
                                ,SB.sb_ffb_block,SB.sb_magic_number);
                        fputs(Content,Dot);
                        fputs("}", Dot);
                        fclose(Dot);
                        createDirectory(DirectoryPath,PathDirectoryLength);
                        char GraphCompileCommand[128];
                        memset(GraphCompileCommand,'\0',128);
                        sprintf(GraphCompileCommand, "dot -Tps source.dot -o %s", (*REP).path);
                        system(GraphCompileCommand);
                        char OpenFileCommand[64];
                        memset(OpenFileCommand,'\0',64);
                        sprintf(OpenFileCommand, "evince  %s", (*REP).path);
                        system(OpenFileCommand);
                    }
                    char LSREPORT[14];
                    memset(LSREPORT,'\0',14);
                    strcpy(LSREPORT,"ls");
                    if(strcmp(LSREPORT,(*REP).name) == 0)
                    {
                        writeLSDirectory((*REP).ruta,SB.sb_p_tree_directory,&SB,&(*disk));
                    }
                    char LOGREPORT[14];
                    memset(LOGREPORT,'\0',14);
                    strcpy(LOGREPORT,"bitacora");
                    if(strcmp(LOGREPORT,(*REP).name) == 0)
                    {
                        fseek(disk,SB.sb_p_log,SEEK_SET);
                        int TotalLogs;
                        fread(&TotalLogs,sizeof(int),1,disk);
                        for(int i = 0; i < TotalLogs; i++)
                        {
                            log LOG;
                            initializeLOG(&LOG);
                            int LogIndex = SB.sb_p_log + sizeof(int) + i * sizeof(log);
                            fseek(disk,LogIndex,SEEK_SET);
                            fread(&LOG,sizeof(log),1,disk);
                            printf(">> B%d\nOperacion: %s\n",i,LOG.log_operation_type);
                            printf("Tipo: %c\n",LOG.log_type);
                            printf("Nombre: %s\n",LOG.log_name);
                            printf("Contenido: %s\n",LOG.log_content);
                            printf("Fecha: %s\n",LOG.log_date);
                        }
                    }
                    SB.sb_montage_count = SB.sb_montage_count + 1;
                    time_t Time = time(0);
                    struct tm *tlocal = localtime(&Time);
                    char TemporalTime[24];
                    strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                    strcpy(SB.sb_date_last_montage,TemporalTime);
                    fseek(disk,PartitionStartIndex,SEEK_SET);
                    fwrite(&SB,sizeof(sb),1,disk);
                }
                else
                {
                    printf(">> Particion corrupta.\n");
                }
            }
            else
            {
                printf(">> La particion no existe.\n");
            }
            fclose(disk);
        }
        else
        {
            printf(">> El disco no existe.\n");
        }
    }
    else
    {
        printf(">> La particion no se encuentra montada.\n");
    }
}

void verifyREPData(rep *REP, int *ValidREP)
{
    if((*ValidREP) != 0)
    {
        int ValidFinalPath = 0;
        for(int i = 0; i < 96; i++)
        {
            if((*REP).path[i] != '\0')
                ValidFinalPath = 1;
        }
        if(ValidFinalPath == 1)
        {
            ValidFinalPath = 0;
            for(int i = 0; i < 6; i++)
            {
                if((*REP).id[i] != '\0')
                    ValidFinalPath = 1;
            }
            if(ValidFinalPath == 0)
            {
                (*ValidREP) = 0;
                printf(">> No se definio un ID.\n");
            }
        }
        else
        {
            (*ValidREP) = 0;
            printf(">> No se definio una PATH.\n");
        }
    }
}

int getREPId(int StartIndex, rep *REP, int *ValidREP)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidREP) = 0;
                printf(">>> El valor de ID tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 16)
            {
                (*REP).id[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de ID es ");
        printf("%s",(*REP).id);
        printf(".\n");
    }
    else
    {
        (*ValidREP) = 0;
        printf(">>> El valor de ID tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

int getREPRuta(int StartIndex, rep *REP, int *ValidREP)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*REP).ruta[j] = Command[i];
            j++;
        }
        printf(">>> El valor de RUTA es ");
        printf("%s",(*REP).ruta);
        printf(".\n");
    }
    else
    {
        (*ValidREP) = 0;
        printf(">>> El valor de RUTA no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

int getREPPath(int StartIndex, rep *REP, int *ValidREP)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*REP).path[j] = Command[i];
            j++;
        }
        printf(">>> El valor de PATH es ");
        printf("%s",(*REP).path);
        printf(".\n");
    }
    else
    {
        (*ValidREP) = 0;
        printf(">>> El valor de PATH no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

int getREPName(int StartIndex, rep *REP, int *ValidREP)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidREP) = 0;
                printf(">>> El valor de NAME tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 14)
            {
                (*REP).name[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de NAME es ");
        printf("%s",(*REP).name);
        printf(".\n");
    }
    else
    {
        (*ValidREP) = 0;
        printf(">>> El valor de NAME tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

void getREPParameters(int StartIndex, rep *REP, int *ValidREP)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char RUTAParameter[4] = "ruta";
            if(compareArray(i,RUTAParameter,4) == 1)
            {
                printf(">> Parametro RUTA reconocido.\n");
                i = getREPRuta(i+6, &(*REP), &(*ValidREP));
            }
            char NAMEParameter[4] = "name";
            if(compareArray(i,NAMEParameter,4) == 1)
            {
                printf(">> Parametro NAME reconocido.\n");
                i = getREPName(i+6, &(*REP), &(*ValidREP));
            }
            char PATHParameter[4] = "path";
            if(compareArray(i,PATHParameter,4) == 1)
            {
                printf(">> Parametro PATH reconocido.\n");
                i = getREPPath(i+6, &(*REP), &(*ValidREP));
            }
            char IDParameter[2] = "id";
            if(compareArray(i,IDParameter,2) == 1)
            {
                printf(">> Parametro ID reconocido.\n");
                i = getREPId(i+4, &(*REP), &(*ValidREP));
            }
        }
    }
}

void initializeREP(rep *REP)
{
    memset((*REP).path,'\0',96);
    memset((*REP).ruta,'\0',96);
    memset((*REP).name,'\0',14);
    memset((*REP).id,'\0',6);
}

void executeREP(int StartIndex)
{
    int ValidREP = 1;
    rep REP;
    initializeREP(&REP);
    getREPParameters(StartIndex,&REP,&ValidREP);
    if(ValidREP == 0)
    {
        printf("> No se puede ejecutar REP porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyREPData(&REP,&ValidREP);
        if(ValidREP == 0)
        {
            printf("> No se puede ejecutar REP por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando REP con los parametros definidos.\n");
            runREP(&REP);
        }
    }
}

void createInnerFile(char *Path, char P, int VirtualTreePointer, sb *SB, FILE *Disk, _mkfile *MKFILE)
{
    int EndDirectory = -1;
    int SlashCounter = 0;
    int FatherDirectory = 0;
    for(int i = 0; i < 96; i++)
    {
        if(Path[i] == '/')
        {
            SlashCounter++;
            if(SlashCounter == 2)
            {
                FatherDirectory = 1;
                EndDirectory = i;
                i = 96;
            }
        }
    }
    if(EndDirectory == -1)
    {
        for(int i = 95; i >= 0; i--)
        {
            if(Path[i] != '\0')
            {
                EndDirectory = i + 1;
                i = -1;
            }
        }
    }
    char Directory[16];
    memset(Directory,'\0',16);
    int j = 0;
    for(int i = 1; i < EndDirectory; i++)
    {
        Directory[j] = Path[i];
        j++;
    }
    if(FatherDirectory == 1)
    {
        if((*SB).sb_virtual_tree_free > 0)
        {

            tvd TVD;
            initializeTVD(&TVD);
            fseek(Disk,VirtualTreePointer,SEEK_SET);
            fread(&TVD,sizeof(tvd),1,Disk);
            int DirectoryFounded = -1;
            int SearchingDirectory = 1;
            int TemporalVirtualTreePointer = VirtualTreePointer;
            do{
                for(int i = 0; i < 6; i++)
                {
                    if(TVD.tvd_p_subdirectorys[i] != -1)
                    {
                        tvd TemporalTVD;
                        initializeTVD(&TemporalTVD);
                        fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                        fread(&TemporalTVD,sizeof(tvd),1,Disk);
                        int NameMatchCounter = 0;
                        for(int j = 0; j < 16; j++)
                        {
                            if(Directory[j] == TemporalTVD.tvd_directory_name[j])
                                NameMatchCounter++;
                        }
                        if(NameMatchCounter == 16)
                        {
                            DirectoryFounded = TVD.tvd_p_subdirectorys[i];
                        }
                    }
                }
                if(DirectoryFounded == -1)
                {
                    if(TVD.tvd_p_tvd != -1)
                    {
                        VirtualTreePointer = TVD.tvd_p_tvd;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fread(&TVD,sizeof(tvd),1,Disk);
                    }
                    else
                    {
                        SearchingDirectory = 0;
                    }
                }
                else
                {
                    SearchingDirectory = 0;
                }
            }while(SearchingDirectory);
            if(DirectoryFounded != -1)
            {
                char NewPath[96];
                memset(NewPath,'\0',96);
                int j = 0;
                for(int i = EndDirectory; i < 96; i++)
                {
                    NewPath[j] = Path[i];
                    j++;
                }
                createInnerFile(NewPath,P,DirectoryFounded,&(*SB),&(*Disk),&(*MKFILE));
            }
            else
            {
                if(P == '1')
                {
                    VirtualTreePointer = TemporalVirtualTreePointer;
                    tvd TVD;
                    initializeTVD(&TVD);
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                    int Showing = 1;
                    do
                    {
                        printf("%s: ",TVD.tvd_directory_name);
                        for(int i = 0; i < 6; i++)
                        {
                            printf("%d ",TVD.tvd_p_subdirectorys[i]);
                        }
                        printf("\n");
                        if(TVD.tvd_p_tvd != -1)
                        {
                            fseek(Disk,TVD.tvd_p_tvd,SEEK_SET);
                            fread(&TVD,sizeof(tvd),1,Disk);
                        }
                        else
                        {
                            Showing = 0;
                        }
                    }while (Showing);
                    initializeTVD(&TVD);
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                    int NewDirectoryIndex = -1;
                    int SearchingEmptySpace = 1;
                    do{
                        for(int i = 0; i < 6; i++)
                        {
                            if(TVD.tvd_p_subdirectorys[i] == -1)
                            {
                                NewDirectoryIndex = i;
                                i = 6;
                            }
                        }
                        if(NewDirectoryIndex == -1)
                        {
                            if(TVD.tvd_p_tvd != -1)
                            {
                                VirtualTreePointer = TVD.tvd_p_tvd;
                                fseek(Disk,VirtualTreePointer,SEEK_SET);
                                fread(&TVD,sizeof(tvd),1,Disk);
                            }
                            else
                            {
                                SearchingEmptySpace = 0;
                            }
                        }
                        else
                        {
                            SearchingEmptySpace = 0;
                        }
                    }while(SearchingEmptySpace);
                    if(NewDirectoryIndex != -1)
                    {
                        int NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                        TVD.tvd_p_subdirectorys[NewDirectoryIndex] = NewDirectoryPointer;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        initializeTVD(&TVD);
                        strcpy(TVD.tvd_directory_name,Directory);
                        time_t Time = time(0);
                        struct tm *tlocal = localtime(&Time);
                        char TemporalTime[24];
                        strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                        strcpy(TVD.tvd_creation_date,TemporalTime);
                        TVD.tvd_proprietary = 1;
                        int BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                        fseek(Disk,BitmapPointer,SEEK_SET);
                        char BitmapContent = '1';
                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                        fseek(Disk,NewDirectoryPointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                        (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                        (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                        char NewPath[96];
                        memset(NewPath,'\0',96);
                        int j = 0;
                        for(int i = EndDirectory; i < 96; i++)
                        {
                            NewPath[j] = Path[i];
                            j++;
                        }
                        createInnerFile(NewPath,P,NewDirectoryPointer,&(*SB),&(*Disk),&(*MKFILE));
                    }
                    else
                    {
                        int NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                        TVD.tvd_p_tvd= NewDirectoryPointer;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        for(int i = 0; i < 6; i++)
                        {
                            TVD.tvd_p_subdirectorys[i] = -1;
                        }
                        TVD.tvd_p_tvd = -1;
                        int BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                        fseek(Disk,BitmapPointer,SEEK_SET);
                        char BitmapContent = '1';
                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                        fseek(Disk,NewDirectoryPointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                        (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                        (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                        VirtualTreePointer = NewDirectoryPointer;
                        if((*SB).sb_virtual_tree_free > 0)
                        {
                            NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                            TVD.tvd_p_subdirectorys[0] = NewDirectoryPointer;
                            fseek(Disk,VirtualTreePointer,SEEK_SET);
                            fwrite(&TVD,sizeof(tvd),1,Disk);
                            initializeTVD(&TVD);
                            strcpy(TVD.tvd_directory_name,Directory);
                            time_t Time = time(0);
                            struct tm *tlocal = localtime(&Time);
                            char TemporalTime[24];
                            strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                            strcpy(TVD.tvd_creation_date,TemporalTime);
                            TVD.tvd_proprietary = 1;
                            BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                            fseek(Disk,BitmapPointer,SEEK_SET);
                            BitmapContent = '1';
                            fwrite(&BitmapContent,sizeof(char),1,Disk);
                            fseek(Disk,NewDirectoryPointer,SEEK_SET);
                            fwrite(&TVD,sizeof(tvd),1,Disk);
                            (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                            (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                            (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                            char NewPath[96];
                            memset(NewPath,'\0',96);
                            int j = 0;
                            for(int i = EndDirectory; i < 96; i++)
                            {
                                NewPath[j] = Path[i];
                                j++;
                            }
                            createInnerFile(NewPath,P,NewDirectoryPointer,&(*SB),&(*Disk),&(*MKFILE));
                        }
                        else
                        {
                            printf(">> No hay espacio suficiente para crear un nuevo directorio.\n");
                        }
                    }
                }
                else
                {
                    printf(">> No existe un directorio padre.\n");
                }
            }
        }
        else
        {
            printf(">> No hay espacio suficiente para crear un nuevo directorio.\n");
        }
    }
    else
    {
        tvd TVD;
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        int Showing = 1;
        do
        {
            printf("%s: ",TVD.tvd_directory_name);
            for(int i = 0; i < 6; i++)
            {
                printf("%d ",TVD.tvd_p_subdirectorys[i]);
            }
            printf("\n");
            if(TVD.tvd_p_tvd != -1)
            {
                fseek(Disk,TVD.tvd_p_tvd,SEEK_SET);
                fread(&TVD,sizeof(tvd),1,Disk);
            }
            else
            {
                Showing = 0;
            }
        }while (Showing);
        initializeTVD(&TVD);
        fseek(Disk,VirtualTreePointer,SEEK_SET);
        fread(&TVD,sizeof(tvd),1,Disk);
        if(TVD.tvd_p_detail_directory == -1)
        {
            if((*SB).sb_detail_directory_free > 0)
            {
                int NewDetailDirectoryPointer = (*SB).sb_p_detail_directory + (*SB).sb_ffb_detail_directory * sizeof(dd);
                TVD.tvd_p_detail_directory =  NewDetailDirectoryPointer;
                fseek(Disk,VirtualTreePointer,SEEK_SET);
                fwrite(&TVD,sizeof(tvd),1,Disk);
                dd DD;
                initializeDD(&DD);
                strcpy(DD.files[0].dd_file_name,Directory);
                time_t Time = time(0);
                struct tm *tlocal = localtime(&Time);
                char TemporalTime[24];
                strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                strcpy(DD.files[0].dd_file_creation_date,TemporalTime);
                strcpy(DD.files[0].dd_file_modified_date,TemporalTime);
                if((*SB).sb_inode_free > 0)
                {
                    int NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                    DD.files[0].dd_file_p_inode = NewInodePointer;
                    if((*MKFILE).size != 0)
                    {
                        int ContentSize = (*MKFILE).size;
                        char Content[(*MKFILE).size];
                        memset(Content,'0',(*MKFILE).size);
                        inode INODE;
                        initializeINODE(&INODE);
                        INODE.i_proprietary = 1;
                        int CancelCreateFile = 0;
                        int ContentIndex = 0;
                        do
                        {
                            INODE.i_count = (*SB).sb_inode_count;
                            INODE.i_block_count = 0;
                            for(int i = 0; i < 4; i++)
                            {
                                if(ContentSize > 0 && (*SB).sb_block_free > 0)
                                {
                                    block BLOCK;
                                    initializeBLOCK(&BLOCK);
                                    for(int j = 0; j < 24; j++)
                                    {
                                        if(ContentSize > 0)
                                        {
                                            BLOCK.block_data[j] = Content[ContentIndex];
                                            ContentSize = ContentSize - 1;
                                            ContentIndex++;
                                            INODE.i_file_size = INODE.i_file_size + 1;
                                        }
                                    }
                                    INODE.i_block_count = INODE.i_block_count + 1;
                                    int BlockIndex = (*SB).sb_p_block + (*SB).sb_ffb_block * sizeof(block);
                                    INODE.i_p_block[i] = BlockIndex;
                                    fseek(Disk,BlockIndex,SEEK_SET);
                                    fwrite(&BLOCK,sizeof(block),1,Disk);
                                    int BitmapBlockIndex = (*SB).sb_p_block_bitmap + (*SB).sb_ffb_block * sizeof(char);
                                    char BitmapContent = '1';
                                    fseek(Disk,BitmapBlockIndex,SEEK_SET);
                                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                                    (*SB).sb_ffb_block = (*SB).sb_ffb_block + 1;
                                    (*SB).sb_block_count = (*SB).sb_block_count + 1;
                                    (*SB).sb_block_free = (*SB).sb_block_free - 1;
                                }
                            }
                            if((*SB).sb_inode_free > 0)
                            {
                                if(ContentSize > 0)
                                {
                                    int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                    char BitmapContent = '1';
                                    fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                                    (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                    INODE.i_p_i = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                    fseek(Disk,NewInodePointer,SEEK_SET);
                                    fwrite(&INODE,sizeof(inode),1,Disk);
                                    (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                    (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                    for(int i = 0 ; i < 4; i++)
                                    {
                                        INODE.i_p_block[i] = -1;
                                    }
                                    INODE.i_p_i = -1;
                                    NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                    INODE.i_file_size = 0;
                                }
                                else
                                {
                                    INODE.i_p_i = -1;
                                    fseek(Disk,NewInodePointer,SEEK_SET);
                                    fwrite(&INODE,sizeof(inode),1,Disk);
                                    int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                    char BitmapContent = '1';
                                    fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                                    (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                    (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                    (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                }
                            }
                            else
                            {
                                CancelCreateFile = 1;
                            }
                        }while(ContentSize > 0 && CancelCreateFile == 0);
                        if(ContentSize == 0)
                        {
                            printf(">> Archivo creado correctamente.\n");
                        }
                        else
                        {
                            printf(">> Error al crear el archivo.\n");
                        }
                    }
                    else
                    {
                        int ContentSize = 0;
                        for(int i = 0; i < 128; i++)
                        {
                            if((*MKFILE).content[i] == '\0')
                            {
                                ContentSize = i;
                                i = 128;
                            }
                        }
                        char Content[ContentSize];
                        for(int i = 0; i < ContentSize; i++)
                        {
                            Content[i] = (*MKFILE).content[i];
                        }
                        inode INODE;
                        initializeINODE(&INODE);
                        INODE.i_proprietary = 1;
                        int CancelCreateFile = 0;
                        int ContentIndex = 0;
                        do
                        {
                            INODE.i_count = (*SB).sb_inode_count;
                            INODE.i_block_count = 0;
                            for(int i = 0; i < 4; i++)
                            {
                                if(ContentSize > 0 && (*SB).sb_block_free > 0)
                                {
                                    block BLOCK;
                                    initializeBLOCK(&BLOCK);
                                    for(int j = 0; j < 24; j++)
                                    {
                                        if(ContentSize > 0)
                                        {
                                            BLOCK.block_data[j] = Content[ContentIndex];
                                            ContentSize = ContentSize - 1;
                                            ContentIndex++;
                                            INODE.i_file_size = INODE.i_file_size + 1;
                                        }
                                    }

                                    INODE.i_block_count = INODE.i_block_count + 1;
                                    int BlockIndex = (*SB).sb_p_block + (*SB).sb_ffb_block * sizeof(block);
                                    INODE.i_p_block[i] = BlockIndex;
                                    fseek(Disk,BlockIndex,SEEK_SET);
                                    fwrite(&BLOCK,sizeof(block),1,Disk);
                                    int BitmapBlockIndex = (*SB).sb_p_block_bitmap + (*SB).sb_ffb_block * sizeof(char);
                                    char BitmapContent = '1';
                                    fseek(Disk,BitmapBlockIndex,SEEK_SET);
                                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                                    (*SB).sb_ffb_block = (*SB).sb_ffb_block + 1;
                                    (*SB).sb_block_count = (*SB).sb_block_count + 1;
                                    (*SB).sb_block_free = (*SB).sb_block_free - 1;
                                }
                            }
                            if((*SB).sb_inode_free > 0)
                            {
                                if(ContentSize > 0)
                                {
                                    int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                    char BitmapContent = '1';
                                    fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                                    (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                    INODE.i_p_i = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                    fseek(Disk,NewInodePointer,SEEK_SET);
                                    fwrite(&INODE,sizeof(inode),1,Disk);
                                    (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                    (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                    for(int i = 0 ; i < 4; i++)
                                    {
                                        INODE.i_p_block[i] = -1;
                                    }
                                    INODE.i_p_i = -1;
                                    NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                    INODE.i_file_size = 0;
                                }
                                else
                                {
                                    INODE.i_p_i = -1;
                                    fseek(Disk,NewInodePointer,SEEK_SET);
                                    fwrite(&INODE,sizeof(inode),1,Disk);
                                    int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                    char BitmapContent = '1';
                                    fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                                    (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                    (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                    (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                }
                            }
                            else
                            {
                                CancelCreateFile = 1;
                            }
                        }while(ContentSize > 0 && CancelCreateFile == 0);
                        if(ContentSize == 0)
                        {
                            printf(">> Archivo creado correctamente.\n");
                        }
                        else
                        {
                            printf(">> Error al crear el archivo.\n");
                        }
                    }
                }
                fseek(Disk,NewDetailDirectoryPointer,SEEK_SET);
                fwrite(&DD,sizeof(dd),1,Disk);
                int BitmapPointer = (*SB).sb_p_detail_directory_bitmap + (*SB).sb_ffb_detail_directory * sizeof(char);
                fseek(Disk,BitmapPointer,SEEK_SET);
                char BitmapContent = '1';
                fwrite(&BitmapContent,sizeof(char),1,Disk);
                (*SB).sb_ffb_detail_directory = (*SB).sb_ffb_detail_directory + 1;
                (*SB).sb_detail_directory_count = (*SB).sb_detail_directory_count + 1;
                (*SB).sb_detail_directory_free = (*SB).sb_detail_directory_free - 1;
            }
            else
            {
                printf(">> No hay suficiente espacio para crear el detalle de directorio.");
            }
        }
        else
        {
            dd DD;
            initializeDD(&DD);
            fseek(Disk,TVD.tvd_p_detail_directory,SEEK_SET);
            fread(&DD,sizeof(dd),1,Disk);
            int SearchingFile = 1;
            int FileFounded = 0;
            do
            {
                for(int i = 0; i < 5; i++)
                {
                    int NameMatchCounter = 0;
                    for(int j = 0; j < 16; j++)
                    {
                        if(DD.files[i].dd_file_name[j] == Directory[j])
                            NameMatchCounter++;
                    }
                    if(NameMatchCounter == 16)
                    {
                        FileFounded = 1;
                        SearchingFile = 0;
                    }
                }
                if(DD.dd_p_dd == -1)
                {
                    SearchingFile = 0;
                }
                else
                {
                    fseek(Disk,DD.dd_p_dd,SEEK_SET);
                    fread(&DD,sizeof(dd),1,Disk);
                }
            }while(SearchingFile == 1 && FileFounded == 0);
            if(FileFounded == 0)
            {
                dd DD;
                initializeDD(&DD);
                int DetailDirectoryPointer = TVD.tvd_p_detail_directory;
                fseek(Disk,DetailDirectoryPointer,SEEK_SET);
                fread(&DD,sizeof(dd),1,Disk);
                int SearchingFreeSpot = 1;
                int NewSpotIndex = -1;
                do
                {
                    for(int i = 0; i < 5; i++)
                    {
                        if(DD.files[i].dd_file_name[0] == '\0')
                        {
                            NewSpotIndex = i;
                            i = 5;
                            SearchingFreeSpot = 0;
                        }
                    }
                    if(NewSpotIndex == -1)
                    {
                        if(DD.dd_p_dd != -1)
                        {
                            DetailDirectoryPointer = DD.dd_p_dd;
                            fseek(Disk,DetailDirectoryPointer,SEEK_SET);
                            fread(&DD,sizeof(dd),1,Disk);
                        }
                        else
                        {
                            SearchingFreeSpot = 0;
                        }
                    }
                }while(SearchingFreeSpot);
                if(NewSpotIndex == -1)
                {
                    int NewDetailDirectoryPointer = (*SB).sb_p_detail_directory + (*SB).sb_ffb_detail_directory * sizeof(dd);
                    DD.dd_p_dd =  NewDetailDirectoryPointer;
                    fseek(Disk,DetailDirectoryPointer,SEEK_SET);
                    fwrite(&DD,sizeof(dd),1,Disk);
                    initializeDD(&DD);
                    strcpy(DD.files[0].dd_file_name,Directory);
                    time_t Time = time(0);
                    struct tm *tlocal = localtime(&Time);
                    char TemporalTime[24];
                    strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                    strcpy(DD.files[0].dd_file_creation_date,TemporalTime);
                    strcpy(DD.files[0].dd_file_modified_date,TemporalTime);
                    if((*SB).sb_inode_free > 0)
                    {
                        int NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                        DD.files[0].dd_file_p_inode = NewInodePointer;
                        if((*MKFILE).size != 0)
                        {
                            int ContentSize = (*MKFILE).size;
                            char Content[(*MKFILE).size];
                            memset(Content,'0',(*MKFILE).size);
                            inode INODE;
                            initializeINODE(&INODE);
                            INODE.i_proprietary = 1;
                            int CancelCreateFile = 0;
                            int ContentIndex = 0;
                            do
                            {
                                INODE.i_count = (*SB).sb_inode_count;
                                INODE.i_block_count = 0;
                                for(int i = 0; i < 4; i++)
                                {
                                    if(ContentSize > 0 && (*SB).sb_block_free > 0)
                                    {
                                        block BLOCK;
                                        initializeBLOCK(&BLOCK);
                                        for(int j = 0; j < 24; j++)
                                        {
                                            if(ContentSize > 0)
                                            {
                                                BLOCK.block_data[j] = Content[ContentIndex];
                                                ContentSize = ContentSize - 1;
                                                ContentIndex++;
                                                INODE.i_file_size = INODE.i_file_size + 1;
                                            }
                                        }
                                        INODE.i_block_count = INODE.i_block_count + 1;
                                        int BlockIndex = (*SB).sb_p_block + (*SB).sb_ffb_block * sizeof(block);
                                        INODE.i_p_block[i] = BlockIndex;
                                        fseek(Disk,BlockIndex,SEEK_SET);
                                        fwrite(&BLOCK,sizeof(block),1,Disk);
                                        int BitmapBlockIndex = (*SB).sb_p_block_bitmap + (*SB).sb_ffb_block * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapBlockIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_block = (*SB).sb_ffb_block + 1;
                                        (*SB).sb_block_count = (*SB).sb_block_count + 1;
                                        (*SB).sb_block_free = (*SB).sb_block_free - 1;
                                    }
                                }
                                if((*SB).sb_inode_free > 0)
                                {
                                    if(ContentSize > 0)
                                    {
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        INODE.i_p_i = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                        for(int i = 0 ; i < 4; i++)
                                        {
                                            INODE.i_p_block[i] = -1;
                                        }
                                        INODE.i_p_i = -1;
                                        NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        INODE.i_file_size = 0;
                                    }
                                    else
                                    {
                                        INODE.i_p_i = -1;
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                    }
                                }
                                else
                                {
                                    CancelCreateFile = 1;
                                }
                            }while(ContentSize > 0 && CancelCreateFile == 0);
                            if(ContentSize == 0)
                            {
                                printf(">> Archivo creado correctamente.\n");
                            }
                            else
                            {
                                printf(">> Error al crear el archivo.\n");
                            }
                        }
                        else
                        {
                            int ContentSize = 0;
                            for(int i = 0; i < 128; i++)
                            {
                                if((*MKFILE).content[i] == '\0')
                                {
                                    ContentSize = i;
                                    i = 128;
                                }
                            }
                            char Content[ContentSize];
                            for(int i = 0; i < ContentSize; i++)
                            {
                                Content[i] = (*MKFILE).content[i];
                            }
                            inode INODE;
                            initializeINODE(&INODE);
                            INODE.i_proprietary = 1;
                            int CancelCreateFile = 0;
                            int ContentIndex = 0;
                            do
                            {
                                INODE.i_count = (*SB).sb_inode_count;
                                INODE.i_block_count = 0;
                                for(int i = 0; i < 4; i++)
                                {
                                    if(ContentSize > 0 && (*SB).sb_block_free > 0)
                                    {
                                        block BLOCK;
                                        initializeBLOCK(&BLOCK);
                                        for(int j = 0; j < 24; j++)
                                        {
                                            if(ContentSize > 0)
                                            {
                                                BLOCK.block_data[j] = Content[ContentIndex];
                                                ContentSize = ContentSize - 1;
                                                ContentIndex++;
                                                INODE.i_file_size = INODE.i_file_size + 1;
                                            }
                                        }
                                        INODE.i_block_count = INODE.i_block_count + 1;
                                        int BlockIndex = (*SB).sb_p_block + (*SB).sb_ffb_block * sizeof(block);
                                        INODE.i_p_block[i] = BlockIndex;
                                        fseek(Disk,BlockIndex,SEEK_SET);
                                        fwrite(&BLOCK,sizeof(block),1,Disk);
                                        int BitmapBlockIndex = (*SB).sb_p_block_bitmap + (*SB).sb_ffb_block * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapBlockIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_block = (*SB).sb_ffb_block + 1;
                                        (*SB).sb_block_count = (*SB).sb_block_count + 1;
                                        (*SB).sb_block_free = (*SB).sb_block_free - 1;
                                    }
                                }
                                if((*SB).sb_inode_free > 0)
                                {
                                    if(ContentSize > 0)
                                    {
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        INODE.i_p_i = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                        for(int i = 0 ; i < 4; i++)
                                        {
                                            INODE.i_p_block[i] = -1;
                                        }
                                        INODE.i_p_i = -1;
                                        NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        INODE.i_file_size = 0;
                                    }
                                    else
                                    {
                                        INODE.i_p_i = -1;
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                    }
                                }
                                else
                                {
                                    CancelCreateFile = 1;
                                }
                            }while(ContentSize > 0 && CancelCreateFile == 0);
                            if(ContentSize == 0)
                            {
                                printf(">> Archivo creado correctamente.\n");
                            }
                            else
                            {
                                printf(">> Error al crear el archivo.\n");
                            }
                        }
                    }
                    fseek(Disk,NewDetailDirectoryPointer,SEEK_SET);
                    fwrite(&DD,sizeof(dd),1,Disk);
                    int BitmapPointer = (*SB).sb_p_detail_directory_bitmap + (*SB).sb_ffb_detail_directory * sizeof(char);
                    fseek(Disk,BitmapPointer,SEEK_SET);
                    char BitmapContent = '1';
                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                    (*SB).sb_ffb_detail_directory = (*SB).sb_ffb_detail_directory + 1;
                    (*SB).sb_detail_directory_count = (*SB).sb_detail_directory_count + 1;
                    (*SB).sb_detail_directory_free = (*SB).sb_detail_directory_free - 1;
                }
                else
                {

                    strcpy(DD.files[NewSpotIndex].dd_file_name,Directory);
                    time_t Time = time(0);
                    struct tm *tlocal = localtime(&Time);
                    char TemporalTime[24];
                    strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                    strcpy(DD.files[NewSpotIndex].dd_file_creation_date,TemporalTime);
                    strcpy(DD.files[NewSpotIndex].dd_file_modified_date,TemporalTime);
                    if((*SB).sb_inode_free > 0)
                    {
                        int NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                        DD.files[NewSpotIndex].dd_file_p_inode = NewInodePointer;
                        if((*MKFILE).size != 0)
                        {
                            int ContentSize = (*MKFILE).size;
                            char Content[(*MKFILE).size];
                            memset(Content,'0',(*MKFILE).size);
                            inode INODE;
                            initializeINODE(&INODE);
                            INODE.i_proprietary = 1;
                            int CancelCreateFile = 0;
                            int ContentIndex = 0;
                            do
                            {
                                INODE.i_count = (*SB).sb_inode_count;
                                INODE.i_block_count = 0;
                                for(int i = 0; i < 4; i++)
                                {
                                    if(ContentSize > 0 && (*SB).sb_block_free > 0)
                                    {
                                        block BLOCK;
                                        initializeBLOCK(&BLOCK);
                                        for(int j = 0; j < 24; j++)
                                        {
                                            if(ContentSize > 0)
                                            {
                                                BLOCK.block_data[j] = Content[ContentIndex];
                                                ContentSize = ContentSize - 1;
                                                ContentIndex++;
                                                INODE.i_file_size = INODE.i_file_size + 1;
                                            }
                                        }
                                        INODE.i_block_count = INODE.i_block_count + 1;
                                        int BlockIndex = (*SB).sb_p_block + (*SB).sb_ffb_block * sizeof(block);
                                        INODE.i_p_block[i] = BlockIndex;
                                        fseek(Disk,BlockIndex,SEEK_SET);
                                        fwrite(&BLOCK,sizeof(block),1,Disk);
                                        int BitmapBlockIndex = (*SB).sb_p_block_bitmap + (*SB).sb_ffb_block * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapBlockIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_block = (*SB).sb_ffb_block + 1;
                                        (*SB).sb_block_count = (*SB).sb_block_count + 1;
                                        (*SB).sb_block_free = (*SB).sb_block_free - 1;
                                    }
                                }
                                if((*SB).sb_inode_free > 0)
                                {
                                    if(ContentSize > 0)
                                    {
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        INODE.i_p_i = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                        for(int i = 0 ; i < 4; i++)
                                        {
                                            INODE.i_p_block[i] = -1;
                                        }
                                        INODE.i_p_i = -1;
                                        NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        INODE.i_file_size = 0;
                                    }
                                    else
                                    {
                                        INODE.i_p_i = -1;
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                    }
                                }
                                else
                                {
                                    CancelCreateFile = 1;
                                }
                            }while(ContentSize > 0 && CancelCreateFile == 0);
                            if(ContentSize == 0)
                            {
                                printf(">> Archivo creado correctamente.\n");
                            }
                            else
                            {
                                printf(">> Error al crear el archivo.\n");
                            }
                        }
                        else
                        {
                            int ContentSize = 0;
                            for(int i = 0; i < 128; i++)
                            {
                                if((*MKFILE).content[i] == '\0')
                                {
                                    ContentSize = i;
                                    i = 128;
                                }
                            }
                            char Content[ContentSize];
                            for(int i = 0; i < ContentSize; i++)
                            {
                                Content[i] = (*MKFILE).content[i];
                            }
                            inode INODE;
                            initializeINODE(&INODE);
                            INODE.i_proprietary = 1;
                            int CancelCreateFile = 0;
                            int ContentIndex = 0;
                            do
                            {
                                INODE.i_count = (*SB).sb_inode_count;
                                INODE.i_block_count = 0;
                                for(int i = 0; i < 4; i++)
                                {
                                    if(ContentSize > 0 && (*SB).sb_block_free > 0)
                                    {
                                        block BLOCK;
                                        initializeBLOCK(&BLOCK);
                                        for(int j = 0; j < 24; j++)
                                        {
                                            if(ContentSize > 0)
                                            {
                                                BLOCK.block_data[j] = Content[ContentIndex];
                                                ContentSize = ContentSize - 1;
                                                ContentIndex++;
                                                INODE.i_file_size = INODE.i_file_size + 1;
                                            }
                                        }
                                        INODE.i_block_count = INODE.i_block_count + 1;
                                        int BlockIndex = (*SB).sb_p_block + (*SB).sb_ffb_block * sizeof(block);
                                        INODE.i_p_block[i] = BlockIndex;
                                        fseek(Disk,BlockIndex,SEEK_SET);
                                        fwrite(&BLOCK,sizeof(block),1,Disk);
                                        int BitmapBlockIndex = (*SB).sb_p_block_bitmap + (*SB).sb_ffb_block * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapBlockIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_block = (*SB).sb_ffb_block + 1;
                                        (*SB).sb_block_count = (*SB).sb_block_count + 1;
                                        (*SB).sb_block_free = (*SB).sb_block_free - 1;
                                    }
                                }
                                if((*SB).sb_inode_free > 0)
                                {
                                    if(ContentSize > 0)
                                    {
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        INODE.i_p_i = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                        for(int i = 0 ; i < 4; i++)
                                        {
                                            INODE.i_p_block[i] = -1;
                                        }
                                        INODE.i_p_i = -1;
                                        NewInodePointer = (*SB).sb_p_inode + (*SB).sb_ffb_inode * sizeof(inode);
                                        INODE.i_file_size = 0;
                                    }
                                    else
                                    {
                                        INODE.i_p_i = -1;
                                        fseek(Disk,NewInodePointer,SEEK_SET);
                                        fwrite(&INODE,sizeof(inode),1,Disk);
                                        int BitmapInodeIndex = (*SB).sb_p_inode_bitmap + (*SB).sb_ffb_inode * sizeof(char);
                                        char BitmapContent = '1';
                                        fseek(Disk,BitmapInodeIndex,SEEK_SET);
                                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                                        (*SB).sb_ffb_inode = (*SB).sb_ffb_inode + 1;
                                        (*SB).sb_inode_count = (*SB).sb_inode_count + 1;
                                        (*SB).sb_inode_free = (*SB).sb_inode_free - 1;
                                    }
                                }
                                else
                                {
                                    CancelCreateFile = 1;
                                }
                            }while(ContentSize > 0 && CancelCreateFile == 0);
                            if(ContentSize == 0)
                            {
                                printf(">> Archivo creado correctamente.\n");
                            }
                            else
                            {
                                printf(">> Error al crear el archivo.\n");
                            }
                        }
                    }
                    fseek(Disk,DetailDirectoryPointer,SEEK_SET);
                    fwrite(&DD,sizeof(dd),1,Disk);
                }
            }
            else
            {
                printf(">> Ya existe un archivo con el mismo nombre.\n");
            }
        }
    }
}

void runMKFILE(_mkfile *MKFILE)
{
    int MountedIndex = -1;
    for(int i = 0; i < 256; i++)
    {
        if(MountedPartitions[i].active == '1' && MountedPartitions[i].letter == (*MKFILE).id[2]
                && MountedPartitions[i].number == ((*MKFILE).id[3] - 48))
        {
            MountedIndex = i;
            i = 256;
        }
    }
    if(MountedIndex != -1)
    {
        FILE *disk;
        if((disk = fopen(MountedPartitions[MountedIndex].path,"rb+")))
        {
            mbr MBR;
            fseek(disk,0,SEEK_SET);
            fread(&MBR,sizeof(mbr),1,disk);
            int NameMatch[4];
            NameMatch[0] = 0;
            NameMatch[1] = 0;
            NameMatch[2] = 0;
            NameMatch[3] = 0;
            for(int i = 0; i < 16; i++)
            {
                if(MBR.mbr_partition_1.part_status == '1'
                        && MBR.mbr_partition_1.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[0]++;
                if(MBR.mbr_partition_2.part_status == '1'
                        && MBR.mbr_partition_2.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[1]++;
                if(MBR.mbr_partition_3.part_status == '1'
                        && MBR.mbr_partition_3.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[2]++;
                if(MBR.mbr_partition_4.part_status == '1'
                        && MBR.mbr_partition_4.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[3]++;
            }
            int PartitionStartIndex = 0;
            int PartitionTotalSize = 0;
            if(NameMatch[0] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_1.part_start;
                PartitionTotalSize = MBR.mbr_partition_1.part_size;
            }
            if(NameMatch[1] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_2.part_start;
                PartitionTotalSize = MBR.mbr_partition_2.part_size;
            }
            if(NameMatch[2] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_3.part_start;
                PartitionTotalSize = MBR.mbr_partition_3.part_size;
            }
            if(NameMatch[3] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_4.part_start;
                PartitionTotalSize = MBR.mbr_partition_4.part_size;
            }
            if(NameMatch[0] == 16 || NameMatch[1] == 16 || NameMatch[2] == 16 || NameMatch[3] == 16)
            {
                fseek(disk,PartitionStartIndex,SEEK_SET);
                sb SB;
                initializeSB(&SB);
                fread(&SB,sizeof(sb),1,disk);
                createInnerFile((*MKFILE).path,(*MKFILE).p,SB.sb_p_tree_directory,&SB,&(*disk),&(*MKFILE));
                SB.sb_montage_count = SB.sb_montage_count + 1;
                time_t Time = time(0);
                struct tm *tlocal = localtime(&Time);
                char TemporalTime[24];
                strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                strcpy(SB.sb_date_last_montage,TemporalTime);
                fseek(disk,PartitionStartIndex,SEEK_SET);
                fwrite(&SB,sizeof(sb),1,disk);
                printf("TVD: %d DD: %d INODE: %d BLOCK: %d\n",SB.sb_virtual_tree_count,SB.sb_detail_directory_count
                       ,SB.sb_inode_count,SB.sb_block_count);
                log LOG;
                initializeLOG(&LOG);
                int LogCounter = 1;
                fseek(disk,SB.sb_p_log,SEEK_SET);
                fread(&LogCounter,sizeof(int),1,disk);
                strcpy(LOG.log_operation_type,"mkfile");
                LOG.log_type = '1';
                strcpy(LOG.log_name,(*MKFILE).path);
                strcpy(LOG.log_date,TemporalTime);
                strcpy(LOG.log_content,(*MKFILE).content);
                strcpy(LOG.log_operation, Command);
                int NewLogIndex = SB.sb_p_log + sizeof(int) + LogCounter * sizeof(log);
                fseek(disk,NewLogIndex,SEEK_SET);
                fwrite(&LOG,sizeof(log),1,disk);
                LogCounter = LogCounter + 1;
                fseek(disk,SB.sb_p_log,SEEK_SET);
                fwrite(&LogCounter,sizeof(int),1,disk);
            }
            else
            {
                printf(">> No se encontro la particion en el disco.\n");
            }
            fclose(disk);
        }
        else
        {
            printf(">> El disco no existe.\n");
        }
    }
    else
    {
        printf("La particion no se encuentra montada.\n");
    }
}

void verifyMKFILEData(_mkfile *MKFILE, int *ValidMKFILE)
{
    if((*ValidMKFILE) != 0)
    {
        int ValidFinalPath = 0;
        for(int i = 0; i < 96; i++)
        {
            if((*MKFILE).path[i] != '\0')
                ValidFinalPath = 1;
        }
        if(ValidFinalPath == 1)
        {
            ValidFinalPath = 0;
            for(int i = 0; i < 6; i++)
            {
                if((*MKFILE).id[i] != '\0')
                    ValidFinalPath = 1;
            }
            if(ValidFinalPath == 0)
            {
                (*ValidMKFILE) = 0;
                printf(">> No se definio un ID.\n");
            }
        }
        else
        {
            (*ValidMKFILE) = 0;
            printf(">> No se definio una PATH.\n");
        }
    }
}

int getMKFILEId(int StartIndex, _mkfile *MKFILE, int *ValidMKFILE)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidMKFILE) = 0;
                printf(">>> El valor de ID tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 16)
            {
                (*MKFILE).id[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de ID es ");
        printf("%s",(*MKFILE).id);
        printf(".\n");
    }
    else
    {
        (*ValidMKFILE) = 0;
        printf(">>> El valor de ID tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

int getMKFILEContent(int StartIndex, _mkfile *MKFILE, int *ValidMKFILE)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*MKFILE).content[j] = Command[i];
            j++;
        }
        printf(">>> El valor de CONT es ");
        printf("%s",(*MKFILE).content);
        printf(".\n");
    }
    else
    {
        (*ValidMKFILE) = 0;
        printf(">>> El valor de CONTENT no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

int getMKFILEPath(int StartIndex, _mkfile *MKFILE, int *ValidMKFILE)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*MKFILE).path[j] = Command[i];
            j++;
        }
        printf(">>> El valor de PATH es ");
        printf("%s",(*MKFILE).path);
        printf(".\n");
    }
    else
    {
        (*ValidMKFILE) = 0;
        printf(">>> El valor de PATH no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

int getMKFILESize(int StartIndex, _mkfile *MKFILE, int *ValidMKFILE)
{
    int NumberCounter = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
            if(itsNumber(Command[i])==1)
                NumberCounter++;
            else
            {
                NumberCounter = 0;
                i = 256;
            }
    }
    if(NumberCounter > 0)
    {
        char NumberArray[NumberCounter+1];
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NumberCounter; i++)
        {
            NumberArray[j] = Command[i];
            j++;
        }
        NumberArray[NumberCounter] = '\0';
        int TemporalSize = 0;
        sscanf(NumberArray,"%d",&TemporalSize);
        (*MKFILE).size = TemporalSize;
        printf(">>> El valor de SIZE es ");
        printf("%d",(*MKFILE).size);
        printf(".\n");
    }
    else
    {
        (*ValidMKFILE) = 0;
        printf(">>> El valor de SIZE tiene que ser un numero.\n");
        return 256;
    }
    return StartIndex + NumberCounter;
}

void getMKFILEParameters(int StartIndex, _mkfile *MKFILE, int *ValidMKFILE)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char SIZEParameter[4] = "size";
            if(compareArray(i,SIZEParameter,4) == 1)
            {
                printf(">> Parametro SIZE reconocido.\n");
                i = getMKFILESize(i+6, &(*MKFILE), &(*ValidMKFILE));
            }
            char PATHParameter[4] = "path";
            if(compareArray(i,PATHParameter,4) == 1)
            {
                printf(">> Parametro PATH reconocido.\n");
                i = getMKFILEPath(i+6, &(*MKFILE), &(*ValidMKFILE));
            }
            char CONTParameter[4] = "cont";
            if(compareArray(i,CONTParameter,4) == 1)
            {
                printf(">> Parametro CONT reconocido.\n");
                i = getMKFILEContent(i+6, &(*MKFILE), &(*ValidMKFILE));
            }
            char PParameter[1] = "p";
            if(compareArray(i,PParameter,1) == 1)
            {
                printf(">> Parametro P activado.\n");
                (*MKFILE).p = '1';
            }
            char IDParameter[2] = "id";
            if(compareArray(i,IDParameter,2) == 1)
            {
                printf(">> Parametro ID reconocido.\n");
                i = getMKFILEId(i+4, &(*MKFILE), &(*ValidMKFILE));
            }
        }
    }
}

void initializeMKFILE(_mkfile *MKFILE)
{
    (*MKFILE).size = 0;
    memset((*MKFILE).path,'\0',96);
    for(int i = 0; i < 128; i++)
    {
        (*MKFILE).content[i] = '\0';
    }
    memset((*MKFILE).id,'\0',6);
    (*MKFILE).p = '0';
}

void executeMKFILE(int StartIndex)
{
    int ValidMKFILE = 1;
    _mkfile MKFILE;
    initializeMKFILE(&MKFILE);
    getMKFILEParameters(StartIndex,&MKFILE,&ValidMKFILE);
    if(ValidMKFILE == 0)
    {
        printf("> No se puede ejecutar MKFILE porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyMKFILEData(&MKFILE,&ValidMKFILE);
        if(ValidMKFILE == 0)
        {
            printf("> No se puede ejecutar MKFILE por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando MKFILE con los parametros definidos.\n");
            runMKFILE(&MKFILE);
        }
    }
}

void createInnerDirectory(char *Path, char P, int VirtualTreePointer, sb *SB, FILE *Disk)
{
    int EndDirectory = -1;
    int SlashCounter = 0;
    int FatherDirectory = 0;
    for(int i = 0; i < 96; i++)
    {
        if(Path[i] == '/')
        {
            SlashCounter++;
            if(SlashCounter == 2)
            {
                FatherDirectory = 1;
                EndDirectory = i;
                i = 96;
            }
        }
    }
    if(EndDirectory == -1)
    {
        for(int i = 95; i >= 0; i--)
        {
            if(Path[i] != '\0')
            {
                EndDirectory = i + 1;
                i = -1;
            }
        }
    }
    char Directory[16];
    memset(Directory,'\0',16);
    int j = 0;
    for(int i = 1; i < EndDirectory; i++)
    {
        Directory[j] = Path[i];
        j++;
    }
    if((*SB).sb_virtual_tree_free > 0)
    {
        if(FatherDirectory == 1)
        {
            tvd TVD;
            initializeTVD(&TVD);
            fseek(Disk,VirtualTreePointer,SEEK_SET);
            fread(&TVD,sizeof(tvd),1,Disk);
            int DirectoryFounded = -1;
            int SearchingDirectory = 1;
            int TemporalVirtualTreePointer = VirtualTreePointer;
            do{
                for(int i = 0; i < 6; i++)
                {
                    if(TVD.tvd_p_subdirectorys[i] != -1)
                    {
                        tvd TemporalTVD;
                        initializeTVD(&TemporalTVD);
                        fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                        fread(&TemporalTVD,sizeof(tvd),1,Disk);
                        int NameMatchCounter = 0;
                        for(int j = 0; j < 16; j++)
                        {
                            if(Directory[j] == TemporalTVD.tvd_directory_name[j])
                                NameMatchCounter++;
                        }
                        if(NameMatchCounter == 16)
                        {
                            DirectoryFounded = TVD.tvd_p_subdirectorys[i];
                        }
                    }
                }
                if(DirectoryFounded == -1)
                {
                    if(TVD.tvd_p_tvd != -1)
                    {
                        VirtualTreePointer = TVD.tvd_p_tvd;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fread(&TVD,sizeof(tvd),1,Disk);
                    }
                    else
                    {
                        SearchingDirectory = 0;
                    }
                }
                else
                {
                    SearchingDirectory = 0;
                }
            }while(SearchingDirectory);
            if(DirectoryFounded != -1)
            {
                char NewPath[96];
                memset(NewPath,'\0',96);
                int j = 0;
                for(int i = EndDirectory; i < 96; i++)
                {
                    NewPath[j] = Path[i];
                    j++;
                }
                createInnerDirectory(NewPath,P,DirectoryFounded,&(*SB),&(*Disk));
            }
            else
            {
                if(P == '1')
                {
                    VirtualTreePointer = TemporalVirtualTreePointer;
                    tvd TVD;
                    initializeTVD(&TVD);
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                    int Showing = 1;
                    do
                    {
                        printf("%s: ",TVD.tvd_directory_name);
                        for(int i = 0; i < 6; i++)
                        {
                            printf("%d ",TVD.tvd_p_subdirectorys[i]);
                        }
                        printf("\n");
                        if(TVD.tvd_p_tvd != -1)
                        {
                            fseek(Disk,TVD.tvd_p_tvd,SEEK_SET);
                            fread(&TVD,sizeof(tvd),1,Disk);
                        }
                        else
                        {
                            Showing = 0;
                        }
                    }while (Showing);
                    initializeTVD(&TVD);
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                    int NewDirectoryIndex = -1;
                    int SearchingEmptySpace = 1;
                    do{
                        for(int i = 0; i < 6; i++)
                        {
                            if(TVD.tvd_p_subdirectorys[i] == -1)
                            {
                                NewDirectoryIndex = i;
                                i = 6;
                            }
                        }
                        if(NewDirectoryIndex == -1)
                        {
                            if(TVD.tvd_p_tvd != -1)
                            {
                                VirtualTreePointer = TVD.tvd_p_tvd;
                                fseek(Disk,VirtualTreePointer,SEEK_SET);
                                fread(&TVD,sizeof(tvd),1,Disk);
                            }
                            else
                            {
                                SearchingEmptySpace = 0;
                            }
                        }
                        else
                        {
                            SearchingEmptySpace = 0;
                        }
                    }while(SearchingEmptySpace);
                    if(NewDirectoryIndex != -1)
                    {
                        int NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                        TVD.tvd_p_subdirectorys[NewDirectoryIndex] = NewDirectoryPointer;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        initializeTVD(&TVD);
                        strcpy(TVD.tvd_directory_name,Directory);
                        time_t Time = time(0);
                        struct tm *tlocal = localtime(&Time);
                        char TemporalTime[24];
                        strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                        strcpy(TVD.tvd_creation_date,TemporalTime);
                        TVD.tvd_proprietary = 1;
                        int BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                        fseek(Disk,BitmapPointer,SEEK_SET);
                        char BitmapContent = '1';
                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                        fseek(Disk,NewDirectoryPointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                        (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                        (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                        char NewPath[96];
                        memset(NewPath,'\0',96);
                        int j = 0;
                        for(int i = EndDirectory; i < 96; i++)
                        {
                            NewPath[j] = Path[i];
                            j++;
                        }
                        createInnerDirectory(NewPath,P,NewDirectoryPointer,&(*SB),&(*Disk));
                    }
                    else
                    {
                        int NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                        TVD.tvd_p_tvd= NewDirectoryPointer;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        for(int i = 0; i < 6; i++)
                        {
                            TVD.tvd_p_subdirectorys[i] = -1;
                        }
                        TVD.tvd_p_tvd = -1;
                        int BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                        fseek(Disk,BitmapPointer,SEEK_SET);
                        char BitmapContent = '1';
                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                        fseek(Disk,NewDirectoryPointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                        (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                        (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                        VirtualTreePointer = NewDirectoryPointer;
                        if((*SB).sb_virtual_tree_free > 0)
                        {
                            NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                            TVD.tvd_p_subdirectorys[0] = NewDirectoryPointer;
                            fseek(Disk,VirtualTreePointer,SEEK_SET);
                            fwrite(&TVD,sizeof(tvd),1,Disk);
                            initializeTVD(&TVD);
                            strcpy(TVD.tvd_directory_name,Directory);
                            time_t Time = time(0);
                            struct tm *tlocal = localtime(&Time);
                            char TemporalTime[24];
                            strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                            strcpy(TVD.tvd_creation_date,TemporalTime);
                            TVD.tvd_proprietary = 1;
                            BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                            fseek(Disk,BitmapPointer,SEEK_SET);
                            BitmapContent = '1';
                            fwrite(&BitmapContent,sizeof(char),1,Disk);
                            fseek(Disk,NewDirectoryPointer,SEEK_SET);
                            fwrite(&TVD,sizeof(tvd),1,Disk);
                            (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                            (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                            (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                            char NewPath[96];
                            memset(NewPath,'\0',96);
                            int j = 0;
                            for(int i = EndDirectory; i < 96; i++)
                            {
                                NewPath[j] = Path[i];
                                j++;
                            }
                            createInnerDirectory(NewPath,P,NewDirectoryPointer,&(*SB),&(*Disk));
                        }
                        else
                        {
                            printf(">> No hay espacio suficiente para crear un nuevo directorio.\n");
                        }
                    }
                }
                else
                {
                    printf(">> No existe un directorio padre.\n");
                }
            }
        }
        else
        {
            tvd TVD;
            initializeTVD(&TVD);
            fseek(Disk,VirtualTreePointer,SEEK_SET);
            fread(&TVD,sizeof(tvd),1,Disk);
            int Showing = 1;
            do
            {
                printf("%s: ",TVD.tvd_directory_name);
                for(int i = 0; i < 6; i++)
                {
                    printf("%d ",TVD.tvd_p_subdirectorys[i]);
                }
                printf("\n");
                if(TVD.tvd_p_tvd != -1)
                {
                    fseek(Disk,TVD.tvd_p_tvd,SEEK_SET);
                    fread(&TVD,sizeof(tvd),1,Disk);
                }
                else
                {
                    Showing = 0;
                }
            }while (Showing);
            initializeTVD(&TVD);
            fseek(Disk,VirtualTreePointer,SEEK_SET);
            fread(&TVD,sizeof(tvd),1,Disk);
            int NewDirectoryIndex = -1;
            int SearchingEmptySpace = 1;
            int CancelOperation = 0;
            do{
                for(int i = 0; i < 6; i++)
                {
                    if(TVD.tvd_p_subdirectorys[i] == -1)
                    {
                        NewDirectoryIndex = i;
                        i = 6;
                    }
                    else
                    {
                        tvd TemporalTVD;
                        initializeTVD(&TemporalTVD);
                        fseek(Disk,TVD.tvd_p_subdirectorys[i],SEEK_SET);
                        fread(&TemporalTVD,sizeof(tvd),1,Disk);
                        int NameMatchCounter = 0;
                        for(int j = 0; j < 16; j++)
                        {
                            if(TemporalTVD.tvd_directory_name[j] == Directory[j])
                                NameMatchCounter++;
                        }
                        if(NameMatchCounter == 16)
                            CancelOperation = 1;
                    }
                }
                if(NewDirectoryIndex == -1)
                {
                    if(TVD.tvd_p_tvd != -1)
                    {
                        VirtualTreePointer = TVD.tvd_p_tvd;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fread(&TVD,sizeof(tvd),1,Disk);
                    }
                    else
                    {
                        SearchingEmptySpace = 0;
                    }
                }
                else
                {
                    SearchingEmptySpace = 0;
                }
            }while(SearchingEmptySpace == 1 && CancelOperation == 0);
            if(CancelOperation == 0)
            {
                if(NewDirectoryIndex != -1)
                {
                    int NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                    TVD.tvd_p_subdirectorys[NewDirectoryIndex] = NewDirectoryPointer;
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fwrite(&TVD,sizeof(tvd),1,Disk);
                    initializeTVD(&TVD);
                    strcpy(TVD.tvd_directory_name,Directory);
                    time_t Time = time(0);
                    struct tm *tlocal = localtime(&Time);
                    char TemporalTime[24];
                    strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                    strcpy(TVD.tvd_creation_date,TemporalTime);
                    TVD.tvd_proprietary = 1;
                    int BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                    fseek(Disk,BitmapPointer,SEEK_SET);
                    char BitmapContent = '1';
                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                    fseek(Disk,NewDirectoryPointer,SEEK_SET);
                    fwrite(&TVD,sizeof(tvd),1,Disk);
                    (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                    (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                    (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                }
                else
                {
                    int NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                    TVD.tvd_p_tvd= NewDirectoryPointer;
                    fseek(Disk,VirtualTreePointer,SEEK_SET);
                    fwrite(&TVD,sizeof(tvd),1,Disk);
                    for(int i = 0; i < 6; i++)
                    {
                        TVD.tvd_p_subdirectorys[i] = -1;
                    }
                    TVD.tvd_p_detail_directory = -1;
                    TVD.tvd_p_tvd = -1;
                    int BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                    fseek(Disk,BitmapPointer,SEEK_SET);
                    char BitmapContent = '1';
                    fwrite(&BitmapContent,sizeof(char),1,Disk);
                    fseek(Disk,NewDirectoryPointer,SEEK_SET);
                    fwrite(&TVD,sizeof(tvd),1,Disk);
                    (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                    (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                    (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                    VirtualTreePointer = NewDirectoryPointer;
                    if((*SB).sb_virtual_tree_free > 0)
                    {
                        NewDirectoryPointer = (*SB).sb_p_tree_directory + (*SB).sb_ffb_tree_directory * sizeof(tvd);
                        TVD.tvd_p_subdirectorys[0] = NewDirectoryPointer;
                        fseek(Disk,VirtualTreePointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        initializeTVD(&TVD);
                        strcpy(TVD.tvd_directory_name,Directory);
                        time_t Time = time(0);
                        struct tm *tlocal = localtime(&Time);
                        char TemporalTime[24];
                        strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                        strcpy(TVD.tvd_creation_date,TemporalTime);
                        TVD.tvd_proprietary = 1;
                        BitmapPointer = (*SB).sb_p_tree_directory_bitmap + (*SB).sb_ffb_tree_directory * sizeof(char);
                        fseek(Disk,BitmapPointer,SEEK_SET);
                        BitmapContent = '1';
                        fwrite(&BitmapContent,sizeof(char),1,Disk);
                        fseek(Disk,NewDirectoryPointer,SEEK_SET);
                        fwrite(&TVD,sizeof(tvd),1,Disk);
                        (*SB).sb_ffb_tree_directory = (*SB).sb_ffb_tree_directory + 1;
                        (*SB).sb_virtual_tree_count = (*SB).sb_virtual_tree_count + 1;
                        (*SB).sb_virtual_tree_free = (*SB).sb_virtual_tree_free - 1;
                    }
                    else
                    {
                        printf(">> No hay espacio suficiente para crear un nuevo directorio.\n");
                    }
                }
            }
            else
            {
                printf(">> Ya existe un directorio con este nombre.\n");
            }
        }
    }
    else
    {
        printf(">> No hay espacio suficiente para crear un nuevo directorio.\n");
    }
}

void runMKDIR(_mkdir *MKDIR)
{
    int MountedIndex = -1;
    for(int i = 0; i < 256; i++)
    {
        if(MountedPartitions[i].active == '1' && MountedPartitions[i].letter == (*MKDIR).id[2]
                && MountedPartitions[i].number == ((*MKDIR).id[3] - 48))
        {
            MountedIndex = i;
            i = 256;
        }
    }
    if(MountedIndex != -1)
    {
        FILE *disk;
        if((disk = fopen(MountedPartitions[MountedIndex].path,"rb+")))
        {
            mbr MBR;
            fseek(disk,0,SEEK_SET);
            fread(&MBR,sizeof(mbr),1,disk);
            int NameMatch[4];
            NameMatch[0] = 0;
            NameMatch[1] = 0;
            NameMatch[2] = 0;
            NameMatch[3] = 0;
            for(int i = 0; i < 16; i++)
            {
                if(MBR.mbr_partition_1.part_status == '1'
                        && MBR.mbr_partition_1.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[0]++;
                if(MBR.mbr_partition_2.part_status == '1'
                        && MBR.mbr_partition_2.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[1]++;
                if(MBR.mbr_partition_3.part_status == '1'
                        && MBR.mbr_partition_3.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[2]++;
                if(MBR.mbr_partition_4.part_status == '1'
                        && MBR.mbr_partition_4.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[3]++;
            }
            int PartitionStartIndex = 0;
            int PartitionTotalSize = 0;
            if(NameMatch[0] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_1.part_start;
                PartitionTotalSize = MBR.mbr_partition_1.part_size;
            }
            if(NameMatch[1] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_2.part_start;
                PartitionTotalSize = MBR.mbr_partition_2.part_size;
            }
            if(NameMatch[2] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_3.part_start;
                PartitionTotalSize = MBR.mbr_partition_3.part_size;
            }
            if(NameMatch[3] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_4.part_start;
                PartitionTotalSize = MBR.mbr_partition_4.part_size;
            }
            if(NameMatch[0] == 16 || NameMatch[1] == 16 || NameMatch[2] == 16 || NameMatch[3] == 16)
            {
                fseek(disk,PartitionStartIndex,SEEK_SET);
                sb SB;
                initializeSB(&SB);
                fread(&SB,sizeof(sb),1,disk);
                createInnerDirectory((*MKDIR).path,(*MKDIR).p,SB.sb_p_tree_directory,&SB,&(*disk));
                SB.sb_montage_count = SB.sb_montage_count + 1;
                time_t Time = time(0);
                struct tm *tlocal = localtime(&Time);
                char TemporalTime[24];
                strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                strcpy(SB.sb_date_last_montage,TemporalTime);
                fseek(disk,PartitionStartIndex,SEEK_SET);
                fwrite(&SB,sizeof(sb),1,disk);
                log LOG;
                initializeLOG(&LOG);
                int LogCounter = 0;
                fseek(disk,SB.sb_p_log,SEEK_SET);
                fread(&LogCounter,sizeof(int),1,disk);
                strcpy(LOG.log_operation_type,"mkdir");
                LOG.log_type = '1';
                strcpy(LOG.log_name,(*MKDIR).path);
                strcpy(LOG.log_date,TemporalTime);
                strcpy(LOG.log_operation, Command);
                int NewLogIndex = SB.sb_p_log + sizeof(int) + LogCounter * sizeof(log);
                fseek(disk,NewLogIndex,SEEK_SET);
                fwrite(&LOG,sizeof(log),1,disk);
                LogCounter = LogCounter + 1;
                fseek(disk,SB.sb_p_log,SEEK_SET);
                fwrite(&LogCounter,sizeof(int),1,disk);
            }
            else
            {
                printf(">> No se encontro la particion en el disco.\n");
            }
            fclose(disk);
        }
        else
        {
            printf(">> El disco no existe.\n");
        }
    }
    else
    {
        printf("La particion no se encuentra montada.\n");
    }
}

void verifyMKDIRData(_mkdir *MKDIR, int *ValidMKDIR)
{
    if((*ValidMKDIR) != 0)
    {
        int ValidFinalPath = 0;
        for(int i = 0; i < 96; i++)
        {
            if((*MKDIR).path[i] != '\0')
                ValidFinalPath = 1;
        }
        if(ValidFinalPath == 1)
        {
            ValidFinalPath = 0;
            for(int i = 0; i < 6; i++)
            {
                if((*MKDIR).id[i] != '\0')
                    ValidFinalPath = 1;
            }
            if(ValidFinalPath == 0)
            {
                (*ValidMKDIR) = 0;
                printf(">> No se definio un ID.\n");
            }
        }
        else
        {
            (*ValidMKDIR) = 0;
            printf(">> No se definio una PATH.\n");
        }
    }
}

int getMKDIRId(int StartIndex, _mkdir *MKDIR, int *ValidMKDIR)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidMKDIR) = 0;
                printf(">>> El valor de ID tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 16)
            {
                (*MKDIR).id[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de ID es ");
        printf("%s",(*MKDIR).id);
        printf(".\n");
    }
    else
    {
        (*ValidMKDIR) = 0;
        printf(">>> El valor de ID tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

int getMKDIRPath(int StartIndex, _mkdir *MKDIR, int *ValidMKDIR)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*MKDIR).path[j] = Command[i];
            j++;
        }
        printf(">>> El valor de PATH es ");
        printf("%s",(*MKDIR).path);
        printf(".\n");
    }
    else
    {
        (*ValidMKDIR) = 0;
        printf(">>> El valor de PATH no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

void getMKDIRParameters(int StartIndex, _mkdir *MKDIR, int *ValidMKDIR)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char PATHParameter[4] = "path";
            if(compareArray(i,PATHParameter,4) == 1)
            {
                printf(">> Parametro PATH reconocido.\n");
                i = getMKDIRPath(i+6, &(*MKDIR), &(*ValidMKDIR));
            }
            char IDParameter[2] = "id";
            if(compareArray(i,IDParameter,2) == 1)
            {
                printf(">> Parametro ID reconocido.\n");
                i = getMKDIRId(i+4, &(*MKDIR), &(*ValidMKDIR));
            }
            char PParameter[1] = "p";
            if(compareArray(i,PParameter,1) == 1)
            {
                printf(">> Parametro P activado.\n");
                (*MKDIR).p = '1';
            }
        }
    }
}

void initializeMKDIR(_mkdir *MKDIR)
{
    memset((*MKDIR).id,'\0',6);
    memset((*MKDIR).path,'\0',96);
    (*MKDIR).p = '0';
}

void executeMKDIR(int StartIndex)
{
    int ValidMKDIR = 1;
    _mkdir MKDIR;
    initializeMKDIR(&MKDIR);
    getMKDIRParameters(StartIndex,&MKDIR,&ValidMKDIR);
    if(ValidMKDIR == 0)
    {
        printf("> No se puede ejecutar MKDIR porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyMKDIRData(&MKDIR,&ValidMKDIR);
        if(ValidMKDIR == 0)
        {
            printf("> No se puede ejecutar MKDIR por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando MKDIR con los parametros definidos.\n");
            runMKDIR(&MKDIR);
        }
    }
}

void runMKFS(mkfs *MKFS)
{
    int MountedIndex = 0;
    char MountedPartitionFinded = '0';
    for(int i = 0; i < 256; i++)
    {
        if((MountedPartitions[i].letter == (*MKFS).id[2]) && (MountedPartitions[i].number == ((*MKFS).id[3]-48)))
        {
            MountedIndex = i;
            MountedPartitionFinded = '1';
        }
    }
    if(MountedPartitionFinded == '1')
    {
        FILE *disk;
        if((disk = fopen(MountedPartitions[MountedIndex].path,"rb+")))
        {
            mbr MBR;
            fseek(disk,0,SEEK_SET);
            fread(&MBR,sizeof(mbr),1,disk);
            int NameMatch[4];
            NameMatch[0] = 0;
            NameMatch[1] = 0;
            NameMatch[2] = 0;
            NameMatch[3] = 0;
            for(int i = 0; i < 16; i++)
            {
                if(MBR.mbr_partition_1.part_status == '1'
                        && MBR.mbr_partition_1.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[0]++;
                if(MBR.mbr_partition_2.part_status == '1'
                        && MBR.mbr_partition_2.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[1]++;
                if(MBR.mbr_partition_3.part_status == '1'
                        && MBR.mbr_partition_3.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[2]++;
                if(MBR.mbr_partition_4.part_status == '1'
                        && MBR.mbr_partition_4.part_name[i] == MountedPartitions[MountedIndex].name[i])
                    NameMatch[3]++;
            }
            int PartitionStartIndex = 0;
            int PartitionTotalSize = 0;
            if(NameMatch[0] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_1.part_start;
                PartitionTotalSize = MBR.mbr_partition_1.part_size;
            }
            if(NameMatch[1] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_2.part_start;
                PartitionTotalSize = MBR.mbr_partition_2.part_size;
            }
            if(NameMatch[2] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_3.part_start;
                PartitionTotalSize = MBR.mbr_partition_3.part_size;
            }
            if(NameMatch[3] == 16)
            {
                PartitionStartIndex = MBR.mbr_partition_4.part_start;
                PartitionTotalSize = MBR.mbr_partition_4.part_size;
            }
            if(NameMatch[0] == 16 || NameMatch[1] == 16 || NameMatch[2] == 16 || NameMatch[3] == 16)
            {
                char StartContent = '\0';
                fseek(disk,PartitionStartIndex,SEEK_SET);
                for(int i = PartitionStartIndex; i < PartitionStartIndex + PartitionTotalSize; i++)
                {
                    fwrite(&StartContent,sizeof(char),1,disk);
                }
                sb SB;
                initializeSB(&SB);
                int NameStartIndex = 0;
                int NameEndIndex = 0;
                for(int i = 0; i < 96; i++)
                {
                    if(MountedPartitions[MountedIndex].path[i] == '/')
                        NameStartIndex = i;
                    if(MountedPartitions[MountedIndex].path[i] == '.')
                        NameEndIndex = i;
                }
                int j = 0;
                for(int i = NameStartIndex + 1; i < NameEndIndex; i++)
                {
                    SB.sb_disk_name[j] = MountedPartitions[MountedIndex].path[i];
                    j++;
                }
                int TotalStructures = getTotalStructures(PartitionTotalSize);
                SB.sb_virtual_tree_count = 0;
                SB.sb_detail_directory_count = 0;
                SB.sb_inode_count = 0;
                SB.sb_block_count = 0;
                SB.sb_virtual_tree_free = TotalStructures;
                SB.sb_detail_directory_free = TotalStructures;
                SB.sb_inode_free = (5 * TotalStructures );
                SB.sb_block_free = (20 * TotalStructures);
                time_t Time = time(0);
                struct tm *tlocal = localtime(&Time);
                char TemporalTime[24];
                strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                strcpy(SB.sb_date_creation,TemporalTime);
                strcpy(SB.sb_date_last_montage,TemporalTime);
                SB.sb_montage_count = 1;
                SB.sb_p_tree_directory_bitmap = PartitionStartIndex + sizeof(sb);
                SB.sb_p_tree_directory = SB.sb_p_tree_directory_bitmap + TotalStructures;
                SB.sb_p_detail_directory_bitmap = SB.sb_p_tree_directory + TotalStructures * sizeof(tvd);
                SB.sb_p_detail_directory = SB.sb_p_detail_directory_bitmap + TotalStructures;
                SB.sb_p_inode_bitmap = SB.sb_p_detail_directory + TotalStructures * sizeof(dd);
                SB.sb_p_inode = SB.sb_p_inode_bitmap + TotalStructures;
                SB.sb_p_block_bitmap = SB.sb_p_inode + 5 * TotalStructures * sizeof(inode);
                SB.sb_p_block = SB.sb_p_block_bitmap + TotalStructures;
                SB.sb_p_log = SB.sb_p_block + 20 * TotalStructures * sizeof(block);
                SB.sb_p_backup = SB.sb_p_log + TotalStructures * sizeof(log);
                SB.sb_s_tree_directory = sizeof(tvd);
                SB.sb_s_detail_directory = sizeof(dd);
                SB.sb_s_inode = sizeof(inode);
                SB.sb_s_block = sizeof(block);
                SB.sb_ffb_tree_directory = 0;
                SB.sb_ffb_detail_directory = 0;
                SB.sb_ffb_inode = 0;
                SB.sb_ffb_block = 0;
                SB.sb_magic_number = 201504468;
                fseek(disk,PartitionStartIndex,SEEK_SET);
                fwrite(&SB,sizeof(sb),1,disk);
                fseek(disk,SB.sb_p_backup,SEEK_SET);
                fwrite(&SB,sizeof(sb),1,disk);
                char BitmapContent = '0';
                fseek(disk,SB.sb_p_tree_directory_bitmap,SEEK_SET);
                for(int i = 0; i < TotalStructures; i++)
                {
                    fwrite(&BitmapContent,sizeof(char),1,disk);
                }
                fseek(disk,SB.sb_p_detail_directory_bitmap,SEEK_SET);
                for(int i = 0; i < TotalStructures; i++)
                {
                    fwrite(&BitmapContent,sizeof(char),1,disk);
                }
                fseek(disk,SB.sb_p_inode_bitmap,SEEK_SET);
                for(int i = 0; i < TotalStructures; i++)
                {
                    fwrite(&BitmapContent,sizeof(char),1,disk);
                }
                fseek(disk,SB.sb_p_block_bitmap,SEEK_SET);
                for(int i = 0; i < TotalStructures; i++)
                {
                    fwrite(&BitmapContent,sizeof(char),1,disk);
                }
                tvd TVD;
                initializeTVD(&TVD);
                strcpy(TVD.tvd_creation_date,TemporalTime);
                TVD.tvd_directory_name[0] = '/';
                TVD.tvd_p_detail_directory = SB.sb_p_detail_directory;
                TVD.tvd_p_tvd = -1;
                TVD.tvd_proprietary = 1;
                fseek(disk,SB.sb_p_tree_directory_bitmap,SEEK_SET);
                BitmapContent = '1';
                fwrite(&BitmapContent,sizeof(char),1,disk);
                fseek(disk,SB.sb_p_tree_directory,SEEK_SET);
                fwrite(&TVD,sizeof(tvd),1,disk);
                SB.sb_virtual_tree_count = SB.sb_virtual_tree_count + 1;
                SB.sb_virtual_tree_free = SB.sb_virtual_tree_free - 1;
                SB.sb_ffb_tree_directory = 1;
                dd DD;
                initializeDD(&DD);
                strcpy(DD.files[0].dd_file_name,"users.txt");
                DD.files[0].dd_file_p_inode = SB.sb_p_inode;
                strcpy(DD.files[0].dd_file_creation_date,TemporalTime);
                strcpy(DD.files[0].dd_file_modified_date,TemporalTime);
                DD.dd_p_dd = -1;
                fseek(disk,SB.sb_p_detail_directory_bitmap,SEEK_SET);
                fwrite(&BitmapContent,sizeof(char),1,disk);
                fseek(disk,SB.sb_p_detail_directory,SEEK_SET);
                fwrite(&DD,sizeof(dd),1,disk);
                SB.sb_detail_directory_count = SB.sb_detail_directory_count + 1;
                SB.sb_detail_directory_free = SB.sb_detail_directory_free - 1;
                SB.sb_ffb_detail_directory = 1;
                inode INODE;
                initializeINODE(&INODE);
                INODE.i_count = 0;
                INODE.i_file_size = 2*sizeof(block);
                INODE.i_block_count = 2;
                INODE.i_p_block[0] = SB.sb_p_block;
                INODE.i_p_block[1] = SB.sb_p_block + sizeof(block);
                INODE.i_p_i = -1;
                INODE.i_proprietary = 1;
                fseek(disk,SB.sb_p_inode_bitmap,SEEK_SET);
                fwrite(&BitmapContent,sizeof(char),1,disk);
                fseek(disk,SB.sb_p_inode,SEEK_SET);
                fwrite(&INODE,sizeof(inode),1,disk);
                SB.sb_inode_count = SB.sb_inode_count + 1;
                SB.sb_inode_free =  SB.sb_inode_free - 1;
                SB.sb_ffb_inode = 1;
                block BLOCK1;
                initializeBLOCK(&BLOCK1);
                strcpy(BLOCK1.block_data,"1,G,root \n 1,U,");
                fseek(disk,SB.sb_p_block_bitmap,SEEK_SET);
                fwrite(&BitmapContent,sizeof(char),1,disk);
                fseek(disk,SB.sb_p_block,SEEK_SET);
                fwrite(&BLOCK1,sizeof(block),1,disk);
                SB.sb_block_count = SB.sb_block_count + 1;
                SB.sb_block_free =  SB.sb_block_free - 1;
                SB.sb_ffb_block = 1;
                block BLOCK2;
                initializeBLOCK(&BLOCK2);
                strcpy(BLOCK2.block_data,"root,root,201504468 \n");
                fseek(disk,SB.sb_p_block_bitmap + sizeof(char),SEEK_SET);
                fwrite(&BitmapContent,sizeof(char),1,disk);
                fseek(disk,SB.sb_p_block + sizeof(block),SEEK_SET);
                fwrite(&BLOCK2,sizeof(block),1,disk);
                SB.sb_block_count = SB.sb_block_count + 1;
                SB.sb_block_free =  SB.sb_block_free - 1;
                SB.sb_ffb_block = 2;
                fseek(disk,PartitionStartIndex,SEEK_SET);
                fwrite(&SB,sizeof(sb),1,disk);
                fseek(disk,SB.sb_p_log,SEEK_SET);
                int InitLog = 0;
                fwrite(&InitLog,sizeof(int),1,disk);
                printf(">> Formateo exitoso.\n");
            }
            else
            {
                printf(">> No se encontro la particion en el disco.\n");
            }
            fclose(disk);
        }
        else
        {
            printf(">> El disco no existe.\n");
        }
    }
    else
    {
        printf(">> La particion no ha sido montada.\n");
    }
}

void verifyMKFSData(mkfs *MKFS, int *ValidMKFS)
{
    if((*MKFS).id[0] == '\0')
    {
        (*ValidMKFS) = 0;
    }
}

int getMKFSType(int StartIndex, mkfs *MKFS, int *ValidMKFS)
{
    if((Command[StartIndex] == 'f' &&  Command[StartIndex+1] == 'a' &&
        Command[StartIndex+2] == 's' &&  Command[StartIndex+3] == 't' )||(
                Command[StartIndex] == 'f' &&  Command[StartIndex+1] == 'u' &&
                Command[StartIndex+2] == 'l' &&  Command[StartIndex+3] == 'l' ))
    {
        (*MKFS).type[0] = Command[StartIndex];
        (*MKFS).type[1] = Command[StartIndex+1];
        (*MKFS).type[2] = Command[StartIndex+2];
        (*MKFS).type[3] = Command[StartIndex+3];
        printf(">>> El valor de TYPE es ");
        printf("%s",(*MKFS).type);
        printf(".\n");
    }
    else
    {
        (*ValidMKFS) = 0;
        printf(">>> El valor de TYPE no es valido.\n");
        return 256;
    }
    return StartIndex;
}

int getMKFSId(int StartIndex, mkfs *MKFS, int *ValidMKFS)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidMKFS) = 0;
                printf(">>> El valor de ID tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 16)
            {
                (*MKFS).id[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de ID es ");
        printf("%s",(*MKFS).id);
        printf(".\n");
    }
    else
    {
        (*ValidMKFS) = 0;
        printf(">>> El valor de ID tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

void getMKFSParameters(int StartIndex, mkfs *MKFS, int *ValidMKFS)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char IDParameter[2] = "id";
            if(compareArray(i,IDParameter,2) == 1)
            {
                printf(">> Parametro ID reconocido.\n");
                i = getMKFSId(i+4, &(*MKFS), &(*ValidMKFS));
            }
            char TYPEParameter[4] = "type";
            if(compareArray(i,TYPEParameter,4) == 1)
            {
                printf(">> Parametro TYPE reconocido.\n");
                i = getMKFSType(i+6, &(*MKFS), &(*ValidMKFS));
            }
        }
    }
}

void initializeMKFS(mkfs *MKFS)
{
    memset((*MKFS).id,'\0',6);
    memset((*MKFS).type,'\0',5);
    (*MKFS).type[0] = 'f';
    (*MKFS).type[1] = 'u';
    (*MKFS).type[2] = 'l';
    (*MKFS).type[3] = 'l';
}

void executeMKFS(int StartIndex)
{
    int ValidMKFS = 1;
    mkfs MKFS;
    initializeMKFS(&MKFS);
    getMKFSParameters(StartIndex,&MKFS,&ValidMKFS);
    if(ValidMKFS == 0)
    {
        printf("> No se puede ejecutar MKFS porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyMKFSData(&MKFS,&ValidMKFS);
        if(ValidMKFS == 0)
        {
            printf("> No se puede ejecutar MKFS por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando MKFS con los parametros definidos.\n");
            runMKFS(&MKFS);
        }
    }
}

void runUNMOUNT(unmount *UNMOUNT)
{
    char PartitionFounded = '0';
    for(int i = 0; i < 256; i++)
    {
        if(MountedPartitions[i].active == '1')
        {
            if(MountedPartitions[i].letter == (*UNMOUNT).name[2]
                    && MountedPartitions[i].number == ((*UNMOUNT).name[3] - 48))
            {
                MountedPartitions[i].active = '0';
                printf(">> Particion desmontada.\n");
                PartitionFounded = '1';
            }
        }
    }
    if(PartitionFounded == '1')
    {
        vd TemporalMountedPartitions[256];
        for(int i = 0; i < 256; i++)
        {
            TemporalMountedPartitions[i].active = '0';
            TemporalMountedPartitions[i].letter = '\0';
            TemporalMountedPartitions[i].number = 0;
            memset(TemporalMountedPartitions[i].path,'\0',96);
            memset(TemporalMountedPartitions[i].name,'\0',16);
        }
        int j = 0;
        for(int i = 0; i < 256; i++)
        {
            if(MountedPartitions[i].active == '1')
            {
                TemporalMountedPartitions[j] = MountedPartitions[i];
                j++;
            }
        }
        for(int i = 0; i < 256; i++)
        {
            MountedPartitions[i] = TemporalMountedPartitions[i];
        }
    }
    else
    {
        printf(">> Particion no encontrada, verifique identificador.\n");
    }
}

void verifyUNMOUNTData(unmount *UNMOUNT, int *ValidUNMOUNT)
{
    if((*ValidUNMOUNT) != 0)
    {
        int ValidFinalPath = 0;
        for(int i = 0; i < 16; i++)
        {
            if((*UNMOUNT).name[i] != '\0')
                ValidFinalPath = 1;
        }
        if(ValidFinalPath == 0)
        {
            (*ValidUNMOUNT) = 0;
            printf(">> No se definio un NAME.\n");
        }
    }
}

int getUNMOUNTName(int StartIndex, unmount *UNMOUNT, int *ValidUNMOUNT)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidUNMOUNT) = 0;
                printf(">>> El valor de NAME tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 16)
            {
                (*UNMOUNT).name[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de NAME es ");
        printf("%s",(*UNMOUNT).name);
        printf(".\n");
    }
    else
    {
        (*ValidUNMOUNT) = 0;
        printf(">>> El valor de NAME tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

void getUNMOUNTParameters(int StartIndex, unmount *UNMOUNT, int *ValidUNMOUNT)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char NAMEParameter[2] = "id";
            if(compareArray(i,NAMEParameter,2) == 1)
            {
                printf(">> Parametro NAME reconocido.\n");
                i = getUNMOUNTName(i+4, &(*UNMOUNT), &(*ValidUNMOUNT));
            }
        }
    }
}

void initializeUNMOUNT(unmount *UNMOUNT)
{
    memset((*UNMOUNT).name,'\0',16);
}

void executeUNMOUNT(int StartIndex)
{
    int ValidUNMOUNT = 1;
    unmount UNMOUNT;
    initializeUNMOUNT(&UNMOUNT);
    getUNMOUNTParameters(StartIndex,&UNMOUNT,&ValidUNMOUNT);
    if(ValidUNMOUNT == 0){
        printf("> No se puede ejecutar UNMOUNT porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyUNMOUNTData(&UNMOUNT,&ValidUNMOUNT);
        if(ValidUNMOUNT == 0)
        {
            printf("> No se puede ejecutar UNMOUNT por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando UNMOUNT con los parametros definidos.\n");
            runUNMOUNT(&UNMOUNT);
        }
    }
}

void runMOUNT(mount *MOUNT)
{
    if((*MOUNT).name[0] == '\0' && (*MOUNT).path[0] == '\0')
    {
        printf("> PARTICIONES MONTADAS:\n");
        for(int i = 0; i < 256; i++)
        {
            if(MountedPartitions[i].active == '1')
            {
                printf(">> ID->vd%c%d &path->%s &name->%s \n",MountedPartitions[i].letter,MountedPartitions[i].number,
                       MountedPartitions[i].path, MountedPartitions[i].name);
            }
        }
    }
    else
    {
        char Letter = '\0';
        int Number = 0;
        char CancelMount = '0';
        for(int i = 0 ; i < 256; i++)
        {
            if(MountedPartitions[i].active == '1')
            {
                int PathMatchCounter = 0;
                for(int j = 0; j < 96; j++)
                {
                    if(MountedPartitions[i].path[j] == (*MOUNT).path[j])
                        PathMatchCounter++;
                }
                if(PathMatchCounter == 96)
                {
                    int NameMatchCounter = 0;
                    for(int j = 0; j < 16; j++)
                    {
                        if(MountedPartitions[i].name[j] == (*MOUNT).name[j])
                            NameMatchCounter++;
                    }
                    if(NameMatchCounter == 16)
                    {
                        CancelMount = '1';
                        printf(">> La particion ya se encuentra montada.\n");
                    }
                    else
                    {
                        Letter = MountedPartitions[i].letter;
                        Number = MountedPartitions[i].number + 1;
                    }
                }
            }
        }
        if(CancelMount == '0')
        {
            if(Letter == '\0' && Number == 0)
            {
                for(int i = 0; i < 256; i++)
                {
                    if(MountedPartitions[i].active == '1')
                    {
                        if(MountedPartitions[i].letter > Letter)
                        {
                            Letter = MountedPartitions[i].letter;
                        }
                    }
                }
                if(Letter == '\0')
                {
                    Letter = 'a';
                }
                else
                {
                    Letter = Letter + 1;
                }
                Number = 1;
            }
            for(int i = 0; i < 256; i++)
            {
                if(MountedPartitions[i].active == '0')
                {
                    MountedPartitions[i].active = '1';
                    MountedPartitions[i].letter = Letter;
                    MountedPartitions[i].number = Number;
                    for(int j = 0; j < 96; j++)
                    {
                        MountedPartitions[i].path[j] = (*MOUNT).path[j];
                    }
                    for(int j = 0; j < 16; j++)
                    {
                        MountedPartitions[i].name[j] = (*MOUNT).name[j];
                    }
                    printf(">> Particion: %s montada en vd%c%d.\n",(*MOUNT).name,Letter,Number);
                    i = 256;
                }
            }
        }
    }
}

int getMOUNTName(int StartIndex, mount *MOUNT, int *ValidMOUNT)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidMOUNT) = 0;
                printf(">>> El valor de NAME tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 16)
            {
                (*MOUNT).name[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de NAME es ");
        printf("%s",(*MOUNT).name);
        printf(".\n");
    }
    else
    {
        (*ValidMOUNT) = 0;
        printf(">>> El valor de NAME tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

int getMOUNTPath(int StartIndex, mount *MOUNT, int *ValidMOUNT)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*MOUNT).path[j] = Command[i];
            j++;
        }
        printf(">>> El valor de PATH es ");
        printf("%s",(*MOUNT).path);
        printf(".\n");
    }
    else
    {
        (*ValidMOUNT) = 0;
        printf(">>> El valor de PATH no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

void getMOUNTParameters(int StartIndex, mount *MOUNT, int *ValidMOUNT)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char PATHParameter[4] = "path";
            if(compareArray(i,PATHParameter,4) == 1)
            {
                printf(">> Parametro PATH reconocido.\n");
                i = getMOUNTPath(i+6, &(*MOUNT), &(*ValidMOUNT));
            }
            char NAMEParameter[4] = "name";
            if(compareArray(i,NAMEParameter,4) == 1)
            {
                printf(">> Parametro NAME reconocido.\n");
                i = getMOUNTName(i+6, &(*MOUNT), &(*ValidMOUNT));
            }
        }
    }
}

void initializeMOUNT(mount *MOUNT)
{
    memset((*MOUNT).path,'\0',96);
    memset((*MOUNT).name,'\0',16);
}

void executeMOUNT(int StartIndex)
{
    int ValidMOUNT = 1;
    mount MOUNT;
    initializeMOUNT(&MOUNT);
    getMOUNTParameters(StartIndex,&MOUNT,&ValidMOUNT);
    if(ValidMOUNT == 0){
        printf("> No se puede ejecutar MOUNT porque tiene parametros invalidos.\n");
    }
    else
    {
        if(ValidMOUNT == 0)
        {
            printf("> No se puede ejecutar MOUNT por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando MOUNT con los parametros definidos.\n");
            runMOUNT(&MOUNT);
        }
    }
}

void runFDISK(fdisk *FDISK)
{
    FILE *disk;
    if((disk = fopen((*FDISK).path,"rb+")))
    {
        fseek(disk,0,SEEK_SET);
        mbr MBR;
        fread(&MBR,sizeof(mbr),1,disk);
        if((*FDISK).size != 0)
        {
            if((*FDISK).type == 'p' || (*FDISK).type == 'e')
            {
                int NameMatch[4];
                NameMatch[0] = 0;
                NameMatch[1] = 0;
                NameMatch[2] = 0;
                NameMatch[3] = 0;
                for(int i = 0; i < 16; i++)
                {
                    if((MBR.mbr_partition_1.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_1.part_status == '1')
                        NameMatch[0]++;
                    if((MBR.mbr_partition_2.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_2.part_status == '1')
                        NameMatch[1]++;
                    if((MBR.mbr_partition_3.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_3.part_status == '1')
                        NameMatch[2]++;
                    if((MBR.mbr_partition_4.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_4.part_status == '1')
                        NameMatch[3]++;
                }
                if(NameMatch[0] != 16 && NameMatch[1] != 16 && NameMatch[2] != 16 && NameMatch[3] != 16)
                {
                    int ExtendedPartition = 0;
                    int ActivePartitions = 0;
                    if(MBR.mbr_partition_1.part_status == '1')
                    {
                        ActivePartitions++;
                        if(MBR.mbr_partition_1.part_type == 'e')
                            ExtendedPartition = 1;
                    }
                    if(MBR.mbr_partition_2.part_status == '1')
                    {
                        ActivePartitions++;
                        if(MBR.mbr_partition_2.part_type == 'e')
                            ExtendedPartition = 1;
                    }
                    if(MBR.mbr_partition_3.part_status == '1')
                    {
                        ActivePartitions++;
                        if(MBR.mbr_partition_3.part_type == 'e')
                            ExtendedPartition = 1;
                    }
                    if(MBR.mbr_partition_4.part_status == '1')
                    {
                        ActivePartitions++;
                        if(MBR.mbr_partition_4.part_type == 'e')
                            ExtendedPartition = 1;
                    }
                    if(ActivePartitions < 4)
                    {
                        if((*FDISK).type == 'e' && ExtendedPartition == 1)
                        {
                            printf(">> Ya existe una particion extendida.\n");
                        }
                        else
                        {
                            int SpaceAvailable[4];
                            SpaceAvailable[0] = 0;
                            SpaceAvailable[1] = 0;
                            SpaceAvailable[2] = 0;
                            SpaceAvailable[3] = 0;
                            if(MBR.mbr_partition_1.part_status == '0')
                            {
                                if(MBR.mbr_partition_2.part_status == '1')
                                {
                                    SpaceAvailable[0] = MBR.mbr_partition_2.part_start - sizeof(mbr);
                                }
                                else
                                {
                                    if(MBR.mbr_partition_3.part_status == '1')
                                    {
                                        SpaceAvailable[0] = MBR.mbr_partition_3.part_start - sizeof(mbr);
                                    }
                                    else
                                    {
                                        if(MBR.mbr_partition_4.part_status == '1')
                                        {
                                            SpaceAvailable[0] = MBR.mbr_partition_4.part_start - sizeof(mbr);
                                        }
                                        else
                                        {
                                            SpaceAvailable[0] = MBR.mbr_size - sizeof(mbr);
                                        }
                                    }
                                }
                            }
                            if(MBR.mbr_partition_2.part_status == '0')
                            {
                                if(MBR.mbr_partition_3.part_status == '1')
                                {
                                    SpaceAvailable[1] = MBR.mbr_partition_3.part_start - sizeof(mbr);
                                }
                                else
                                {
                                    if(MBR.mbr_partition_4.part_status == '1')
                                    {
                                        SpaceAvailable[1] = MBR.mbr_partition_4.part_start - sizeof(mbr);
                                    }
                                    else
                                    {
                                        SpaceAvailable[1] = MBR.mbr_size - sizeof(mbr);
                                        if(MBR.mbr_partition_1.part_status == '1')
                                            SpaceAvailable[1] = SpaceAvailable[1] - MBR.mbr_partition_1.part_size;
                                    }
                                }
                            }
                            if(MBR.mbr_partition_3.part_status == '0')
                            {
                                if(MBR.mbr_partition_4.part_status == '1')
                                {
                                    SpaceAvailable[2] = MBR.mbr_partition_4.part_start - sizeof(mbr);
                                }
                                else
                                {
                                    SpaceAvailable[2] = MBR.mbr_size - sizeof(mbr);
                                    if(MBR.mbr_partition_1.part_status == '1')
                                        SpaceAvailable[2] = SpaceAvailable[2] - MBR.mbr_partition_1.part_size;
                                    if(MBR.mbr_partition_2.part_status == '1')
                                        SpaceAvailable[2] = SpaceAvailable[2] - MBR.mbr_partition_2.part_size;
                                }
                            }
                            if(MBR.mbr_partition_4.part_status == '0')
                            {
                                SpaceAvailable[3] = MBR.mbr_size - sizeof(mbr);
                                if(MBR.mbr_partition_1.part_status == '1')
                                    SpaceAvailable[3] = SpaceAvailable[3] - MBR.mbr_partition_1.part_size;
                                if(MBR.mbr_partition_2.part_status == '1')
                                    SpaceAvailable[3] = SpaceAvailable[3] - MBR.mbr_partition_2.part_size;
                                if(MBR.mbr_partition_3.part_status == '1')
                                    SpaceAvailable[3] = SpaceAvailable[3] - MBR.mbr_partition_3.part_size;

                            }
                            int PartitionDestiny = 0;
                            if((*FDISK).fit[0] == 'f')
                            {
                                if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0])
                                {
                                    PartitionDestiny = 1;
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 2;
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 3;
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 4;
                                }
                            }
                            if((*FDISK).fit[0] == 'b')
                            {
                                if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0])
                                {
                                    PartitionDestiny = 1;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1])
                                    {
                                        if(SpaceAvailable[0] <= SpaceAvailable[1])
                                            PartitionDestiny = 1;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 1)
                                    {
                                        if(SpaceAvailable[0] <= SpaceAvailable[2])
                                            PartitionDestiny = 1;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 1)
                                    {
                                        if(SpaceAvailable[0] <= SpaceAvailable[3])
                                            PartitionDestiny = 1;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 2;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0])
                                    {
                                        if(SpaceAvailable[1] <= SpaceAvailable[0])
                                            PartitionDestiny = 2;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 2)
                                    {
                                        if(SpaceAvailable[1] <= SpaceAvailable[2])
                                            PartitionDestiny = 2;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 2)
                                    {
                                        if(SpaceAvailable[1] <= SpaceAvailable[3])
                                            PartitionDestiny = 2;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 3;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1])
                                    {
                                        if(SpaceAvailable[2] <= SpaceAvailable[1])
                                            PartitionDestiny = 3;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0]) && PartitionDestiny == 3)
                                    {
                                        if(SpaceAvailable[2] <= SpaceAvailable[0])
                                            PartitionDestiny = 3;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 3)
                                    {
                                        if(SpaceAvailable[2] <= SpaceAvailable[3])
                                            PartitionDestiny = 3;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 4;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1])
                                    {
                                        if(SpaceAvailable[3] <= SpaceAvailable[1])
                                            PartitionDestiny = 4;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 4)
                                    {
                                        if(SpaceAvailable[3] <= SpaceAvailable[2])
                                            PartitionDestiny = 4;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0]) && PartitionDestiny == 4)
                                    {
                                        if(SpaceAvailable[3] <= SpaceAvailable[0])
                                            PartitionDestiny = 4;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                            }
                            if((*FDISK).fit[0] == 'w')
                            {
                                if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0])
                                {
                                    PartitionDestiny = 1;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1])
                                    {
                                        if(SpaceAvailable[0] >= SpaceAvailable[1])
                                            PartitionDestiny = 1;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 1)
                                    {
                                        if(SpaceAvailable[0] >= SpaceAvailable[2])
                                            PartitionDestiny = 1;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 1)
                                    {
                                        if(SpaceAvailable[0] >= SpaceAvailable[3])
                                            PartitionDestiny = 1;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 2;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0])
                                    {
                                        if(SpaceAvailable[1] >= SpaceAvailable[0])
                                            PartitionDestiny = 2;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 2)
                                    {
                                        if(SpaceAvailable[1] >= SpaceAvailable[2])
                                            PartitionDestiny = 2;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 2)
                                    {
                                        if(SpaceAvailable[1] >= SpaceAvailable[3])
                                            PartitionDestiny = 2;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 3;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1])
                                    {
                                        if(SpaceAvailable[2] >= SpaceAvailable[1])
                                            PartitionDestiny = 3;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0]) && PartitionDestiny == 3)
                                    {
                                        if(SpaceAvailable[2] >= SpaceAvailable[0])
                                            PartitionDestiny = 3;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 3)
                                    {
                                        if(SpaceAvailable[2] >= SpaceAvailable[3])
                                            PartitionDestiny = 3;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                                if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[3]) && PartitionDestiny == 0)
                                {
                                    PartitionDestiny = 4;
                                    if(getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[1])
                                    {
                                        if(SpaceAvailable[3] >= SpaceAvailable[1])
                                            PartitionDestiny = 4;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[2]) && PartitionDestiny == 4)
                                    {
                                        if(SpaceAvailable[3] >= SpaceAvailable[2])
                                            PartitionDestiny = 4;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                    if((getFinalSize((*FDISK).size,(*FDISK).unit) < SpaceAvailable[0]) && PartitionDestiny == 4)
                                    {
                                        if(SpaceAvailable[3] >= SpaceAvailable[0])
                                            PartitionDestiny = 4;
                                        else
                                            PartitionDestiny = 0;
                                    }
                                }
                            }
                            if(PartitionDestiny == 1)
                            {
                                MBR.mbr_partition_1.part_fit = (*FDISK).fit[0];
                                for(int i = 0; i < 16; i++)
                                {
                                    MBR.mbr_partition_1.part_name[i] = (*FDISK).name[i];
                                }
                                MBR.mbr_partition_1.part_size = getFinalSize((*FDISK).size,(*FDISK).unit);
                                MBR.mbr_partition_1.part_start = sizeof(mbr);
                                MBR.mbr_partition_1.part_status = '1';
                                MBR.mbr_partition_1.part_type = (*FDISK).type;
                                printf(">> Particion: %s creada.\n",MBR.mbr_partition_1.part_name);
                                if((*FDISK).type == 'e')
                                {
                                    ebr EBR;
                                    EBR.part_status = '0';
                                    EBR.part_next = -1;
                                    fseek(disk,MBR.mbr_partition_1.part_start,SEEK_SET);
                                    fwrite(&EBR,sizeof(ebr),1,disk);
                                }
                            }
                            if(PartitionDestiny == 2)
                            {
                                MBR.mbr_partition_2.part_fit = (*FDISK).fit[0];
                                for(int i = 0; i < 16; i++)
                                {
                                    MBR.mbr_partition_2.part_name[i] = (*FDISK).name[i];
                                }
                                MBR.mbr_partition_2.part_size = getFinalSize((*FDISK).size,(*FDISK).unit);
                                MBR.mbr_partition_2.part_start = MBR.mbr_partition_1.part_start + MBR.mbr_partition_1.part_size;
                                MBR.mbr_partition_2.part_status = '1';
                                MBR.mbr_partition_2.part_type = (*FDISK).type;
                                printf(">> Particion: %s creada.\n",MBR.mbr_partition_2.part_name);
                                if((*FDISK).type == 'e')
                                {
                                    ebr EBR;
                                    EBR.part_status = '0';
                                    EBR.part_next = -1;
                                    fseek(disk,MBR.mbr_partition_2.part_start,SEEK_SET);
                                    fwrite(&EBR,sizeof(ebr),1,disk);
                                }
                            }
                            if(PartitionDestiny == 3)
                            {
                                MBR.mbr_partition_3.part_fit = (*FDISK).fit[0];
                                for(int i = 0; i < 16; i++)
                                {
                                    MBR.mbr_partition_3.part_name[i] = (*FDISK).name[i];
                                }
                                MBR.mbr_partition_3.part_size = getFinalSize((*FDISK).size,(*FDISK).unit);
                                MBR.mbr_partition_3.part_start = MBR.mbr_partition_2.part_start + MBR.mbr_partition_2.part_size;
                                MBR.mbr_partition_3.part_status = '1';
                                MBR.mbr_partition_3.part_type = (*FDISK).type;
                                printf(">> Particion: %s creada.\n",MBR.mbr_partition_3.part_name);
                                if((*FDISK).type == 'e')
                                {
                                    ebr EBR;
                                    EBR.part_status = '0';
                                    EBR.part_next = -1;
                                    fseek(disk,MBR.mbr_partition_3.part_start,SEEK_SET);
                                    fwrite(&EBR,sizeof(ebr),1,disk);
                                }
                            }
                            if(PartitionDestiny == 4)
                            {
                                MBR.mbr_partition_4.part_fit = (*FDISK).fit[0];
                                for(int i = 0; i < 16; i++)
                                {
                                    MBR.mbr_partition_4.part_name[i] = (*FDISK).name[i];
                                }
                                MBR.mbr_partition_4.part_size = getFinalSize((*FDISK).size,(*FDISK).unit);
                                MBR.mbr_partition_4.part_start = MBR.mbr_partition_3.part_start + MBR.mbr_partition_3.part_size;
                                MBR.mbr_partition_4.part_status = '1';
                                MBR.mbr_partition_4.part_type = (*FDISK).type;
                                printf(">> Particion: %s creada.\n",MBR.mbr_partition_4.part_name);
                                if((*FDISK).type == 'e')
                                {
                                    ebr EBR;
                                    EBR.part_status = '0';
                                    EBR.part_next = -1;
                                    fseek(disk,MBR.mbr_partition_4.part_start,SEEK_SET);
                                    fwrite(&EBR,sizeof(ebr),1,disk);
                                }
                            }
                            if(PartitionDestiny == 0)
                            {
                                printf(">> No hay espacio disponible en el disco.\n");
                            }
                        }
                    }
                    else
                    {
                        printf(">> Todas las particiones se encuentran activas.\n");
                    }
                }
                else
                {
                    printf(">> Ya existe una particion con este nombre.\n");
                }
            }
            else
            {
                printf(">> En construccion :).\n");
            }
        }
        if((*FDISK)._delete[0] != '\0')
        {
            if((*FDISK).type == 'p' || (*FDISK).type == 'e')
            {
                int NameMatch[4];
                NameMatch[0] = 0;
                NameMatch[1] = 0;
                NameMatch[2] = 0;
                NameMatch[3] = 0;
                for(int i = 0; i < 16; i++)
                {
                    if((MBR.mbr_partition_1.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_1.part_status == '1')
                        NameMatch[0]++;
                    if((MBR.mbr_partition_2.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_2.part_status == '1')
                        NameMatch[1]++;
                    if((MBR.mbr_partition_3.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_3.part_status == '1')
                        NameMatch[2]++;
                    if((MBR.mbr_partition_4.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_4.part_status == '1')
                        NameMatch[3]++;
                }
                if(NameMatch[0] == 16)
                {
                    printf(">> Particion encontrada, confirme su eliminacion (S):\n");
                    char Confirm[3];
                    fgets(Confirm,3,stdin);
                    if(Confirm[0] == 's' || Confirm[0] == 'S')
                    {
                        MBR.mbr_partition_1.part_status = '0';
                        if((*FDISK)._delete[1] == 'u')
                        {
                            char DeleteContent = '\0';
                            fseek(disk,MBR.mbr_partition_1.part_start,SEEK_SET);
                            for(int i = 0; i < MBR.mbr_partition_1.part_size; i++)
                            {
                                fwrite(&DeleteContent,sizeof(DeleteContent),1,disk);
                            }
                        }
                        printf(">> Particion eliminada.\n");
                    }
                    else
                    {
                        printf(">> Eliminacion cancelada.\n");
                    }
                }
                if(NameMatch[1] == 16)
                {
                    printf(">> Particion encontrada, confirme su eliminacion (S):\n");
                    char Confirm[3];
                    fgets(Confirm,3,stdin);
                    if(Confirm[0] == 's' || Confirm[0] == 'S')
                    {
                        MBR.mbr_partition_2.part_status = '0';
                        if((*FDISK)._delete[1] == 'u')
                        {
                            char DeleteContent = '\0';
                            fseek(disk,MBR.mbr_partition_2.part_start,SEEK_SET);
                            for(int i = 0; i < MBR.mbr_partition_2.part_size; i++)
                            {
                                fwrite(&DeleteContent,sizeof(DeleteContent),1,disk);
                            }
                        }
                        printf(">> Particion eliminada.\n");
                    }
                    else
                    {
                        printf(">> Eliminacion cancelada.\n");
                    }
                }
                if(NameMatch[2] == 16)
                {
                    printf(">> Particion encontrada, confirme su eliminacion (S):\n");
                    char Confirm[3];
                    fgets(Confirm,3,stdin);
                    if(Confirm[0] == 's' || Confirm[0] == 'S')
                    {
                        MBR.mbr_partition_3.part_status = '0';
                        if((*FDISK)._delete[1] == 'u')
                        {
                            char DeleteContent = '\0';
                            fseek(disk,MBR.mbr_partition_3.part_start,SEEK_SET);
                            for(int i = 0; i < MBR.mbr_partition_3.part_size; i++)
                            {
                                fwrite(&DeleteContent,sizeof(DeleteContent),1,disk);
                            }
                        }
                        printf(">> Particion eliminada.\n");
                    }
                    else
                    {
                        printf(">> Eliminacion cancelada.\n");
                    }
                }
                if(NameMatch[3] == 16)
                {
                    printf(">> Particion encontrada, confirme su eliminacion (S):\n");
                    char Confirm[3];
                    fgets(Confirm,3,stdin);
                    if(Confirm[0] == 's' || Confirm[0] == 'S')
                    {
                        MBR.mbr_partition_4.part_status = '0';
                        if((*FDISK)._delete[1] == 'u')
                        {
                            char DeleteContent = '\0';
                            fseek(disk,MBR.mbr_partition_4.part_start,SEEK_SET);
                            for(int i = 0; i < MBR.mbr_partition_4.part_size; i++)
                            {
                                fwrite(&DeleteContent,sizeof(DeleteContent),1,disk);
                            }
                        }
                        printf(">> Particion eliminada.\n");
                    }
                    else
                    {
                        printf(">> Eliminacion cancelada.\n");
                    }
                }
                if(NameMatch[0] != 16 && NameMatch[1] != 16 && NameMatch[2] != 16 && NameMatch[3] != 16)
                {
                    printf(">> Particion no encontrada.\n");
                }
            }
            else
            {
                printf(">> En construccion :).\n");
            }
        }
        if((*FDISK).add != 0)
        {
            if((*FDISK).type == 'p' || (*FDISK).type == 'e')
            {
                int NameMatch[4];
                NameMatch[0] = 0;
                NameMatch[1] = 0;
                NameMatch[2] = 0;
                NameMatch[3] = 0;
                for(int i = 0; i < 16; i++)
                {
                    if((MBR.mbr_partition_1.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_1.part_status == '1')
                        NameMatch[0]++;
                    if((MBR.mbr_partition_2.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_2.part_status == '1')
                        NameMatch[1]++;
                    if((MBR.mbr_partition_3.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_3.part_status == '1')
                        NameMatch[2]++;
                    if((MBR.mbr_partition_4.part_name[i] == (*FDISK).name[i]) && MBR.mbr_partition_4.part_status == '1')
                        NameMatch[3]++;
                }
                if(NameMatch[0] == 16)
                {
                    int SpaceAvailable = 0;
                    if(MBR.mbr_partition_2.part_status == '1')
                    {
                        SpaceAvailable = MBR.mbr_partition_2.part_start - sizeof(mbr);
                    }
                    else
                    {
                        if(MBR.mbr_partition_3.part_status == '1')
                        {
                            SpaceAvailable = MBR.mbr_partition_3.part_start - sizeof(mbr);
                        }
                        else
                        {
                            if(MBR.mbr_partition_4.part_status == '1')
                            {
                                SpaceAvailable = MBR.mbr_partition_4.part_start - sizeof(mbr);
                            }
                            else
                            {
                                SpaceAvailable = MBR.mbr_size - sizeof(mbr);
                            }
                        }
                    }
                    if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_1.part_size) < SpaceAvailable)
                    {
                        if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_1.part_size) > 0)
                        {
                            MBR.mbr_partition_1.part_size = getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_1.part_size;
                            printf(">> El tamaño de la particion se actualizo correctamente.\n");
                        }
                        else
                        {
                            printf(">> El tamaño de la particion tiene que ser positivo.\n");
                        }
                    }
                    else
                    {
                        printf(">> No hay suficiente espacio disponible.\n");
                    }
                }
                if(NameMatch[1] == 16)
                {
                    int SpaceAvailable = 0;
                    if(MBR.mbr_partition_3.part_status == '1')
                    {
                        SpaceAvailable = MBR.mbr_partition_3.part_start - sizeof(mbr);
                    }
                    else
                    {
                        if(MBR.mbr_partition_4.part_status == '1')
                        {
                            SpaceAvailable = MBR.mbr_partition_4.part_start - sizeof(mbr);
                        }
                        else
                        {
                            SpaceAvailable = MBR.mbr_size - sizeof(mbr);
                            if(MBR.mbr_partition_1.part_status == '1')
                                SpaceAvailable = SpaceAvailable - MBR.mbr_partition_1.part_size;
                        }
                    }
                    if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_2.part_size) < SpaceAvailable)
                    {
                        if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_2.part_size) > 0)
                        {
                            MBR.mbr_partition_2.part_size = getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_2.part_size;
                            printf(">> El tamaño de la particion se actualizo correctamente.\n");
                        }
                        else
                        {
                            printf(">> El tamaño de la particion tiene que ser positivo.\n");
                        }
                    }
                    else
                    {
                        printf(">> No hay suficiente espacio disponible.\n");
                    }
                }
                if(NameMatch[2] == 16)
                {
                    int SpaceAvailable = 0;
                    if(MBR.mbr_partition_4.part_status == '1')
                    {
                        SpaceAvailable = MBR.mbr_partition_4.part_start - sizeof(mbr);
                    }
                    else
                    {
                        SpaceAvailable = MBR.mbr_size - sizeof(mbr);
                        if(MBR.mbr_partition_1.part_status == '1')
                            SpaceAvailable = SpaceAvailable - MBR.mbr_partition_1.part_size;
                        if(MBR.mbr_partition_2.part_status == '1')
                            SpaceAvailable = SpaceAvailable - MBR.mbr_partition_2.part_size;
                    }
                    if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_3.part_size) < SpaceAvailable)
                    {
                        if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_3.part_size) > 0)
                        {
                            MBR.mbr_partition_3.part_size = getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_3.part_size;
                            printf(">> El tamaño de la particion se actualizo correctamente.\n");
                        }
                        else
                        {
                            printf(">> El tamaño de la particion tiene que ser positivo.\n");
                        }
                    }
                    else
                    {
                        printf(">> No hay suficiente espacio disponible.\n");
                    }
                }
                if(NameMatch[3] == 16)
                {
                    int SpaceAvailable = 0;

                    SpaceAvailable = MBR.mbr_partition_4.part_start - sizeof(mbr);
                    SpaceAvailable = MBR.mbr_size - sizeof(mbr);
                    if(MBR.mbr_partition_1.part_status == '1')
                        SpaceAvailable = SpaceAvailable - MBR.mbr_partition_1.part_size;
                    if(MBR.mbr_partition_2.part_status == '1')
                        SpaceAvailable = SpaceAvailable - MBR.mbr_partition_2.part_size;
                    if(MBR.mbr_partition_3.part_status == '1')
                        SpaceAvailable = SpaceAvailable - MBR.mbr_partition_3.part_size;
                    if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_4.part_size) < SpaceAvailable)
                    {
                        if((getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_4.part_size) > 0)
                        {
                            MBR.mbr_partition_4.part_size = getFinalSize((*FDISK).add,(*FDISK).unit) + MBR.mbr_partition_4.part_size;
                            printf(">> El tamaño de la particion se actualizo correctamente.\n");
                        }
                        else
                        {
                            printf(">> El tamaño de la particion tiene que ser positivo.\n");
                        }
                    }
                    else
                    {
                        printf(">> No hay suficiente espacio disponible.\n");
                    }
                }
                if(NameMatch[0] != 16 && NameMatch[1] != 16 && NameMatch[2] != 16 && NameMatch[3] != 16)
                {
                    printf(">> Particion no encontrada.\n");
                }
            }
            else
            {
                printf(">> En construccion :).\n");
            }
        }
        fseek(disk,0,SEEK_SET);
        fwrite(&MBR,sizeof(mbr),1,disk);
        fclose(disk);
    }
    else
    {
        printf(">> No existe el disco a operar.\n");
    }
}

void verifyFDISKData(fdisk *FDISK, int *ValidFDISK)
{
    if((*FDISK).size == 0)
    {
        if((*FDISK)._delete[0] == '\0' && (*FDISK).add == 0)
        {
            (*ValidFDISK) = 0;
            printf(">> No se definio el SIZE.\n");
        }
    }
    if((*ValidFDISK) != 0)
    {
        int ValidFinalPath = 0;
        for(int i = 0; i < 96; i++)
        {
            if((*FDISK).path[i] != '\0')
                ValidFinalPath = 1;
        }
        if(ValidFinalPath == 1)
        {
            ValidFinalPath = 0;
            for(int i = 0; i < 16; i++)
            {
                if((*FDISK).name[i] != '\0')
                    ValidFinalPath = 1;
            }
            if(ValidFinalPath == 0)
            {
                (*ValidFDISK) = 0;
                printf(">> No se definio un NAME.\n");
            }
        }
        else
        {
            (*ValidFDISK) = 0;
            printf(">> No se definio una PATH.\n");
        }
    }
}

int getFDISKAdd(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    int NumberCounter = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
            if((itsNumber(Command[i])==1) || Command[i] == '-')
                NumberCounter++;
            else
            {
                NumberCounter = 0;
                i = 256;
            }
    }
    if(NumberCounter > 0)
    {
        char NumberArray[NumberCounter+1];
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NumberCounter; i++)
        {
            NumberArray[j] = Command[i];
            j++;
        }
        NumberArray[NumberCounter] = '\0';
        int TemporalSize = 0;
        sscanf(NumberArray,"%d",&TemporalSize);
        (*FDISK).add = TemporalSize;
        printf(">>> El valor de ADD es ");
        printf("%d",(*FDISK).add);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de ADD tiene que ser un numero.\n");
        return 256;
    }
    return StartIndex + NumberCounter;
}

int getFDISKDelete(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    if((Command[StartIndex] == 'f' &&  Command[StartIndex+1] == 'a' &&
        Command[StartIndex+2] == 's' &&  Command[StartIndex+3] == 't' )||(
                Command[StartIndex] == 'f' &&  Command[StartIndex+1] == 'u' &&
                Command[StartIndex+2] == 'l' &&  Command[StartIndex+3] == 'l' ))
    {
        (*FDISK)._delete[0] = Command[StartIndex];
        (*FDISK)._delete[1] = Command[StartIndex+1];
        (*FDISK)._delete[2] = Command[StartIndex+2];
        (*FDISK)._delete[3] = Command[StartIndex+3];
        printf(">>> El valor de DELETE es ");
        printf("%s",(*FDISK)._delete);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de DELETE no es valido.\n");
        return 256;
    }
    return StartIndex;
}

int getFDISKFit(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    if((Command[StartIndex] == 'b' &&  Command[StartIndex+1] == 'f') ||
            (Command[StartIndex] == 'f' &&  Command[StartIndex+1] == 'f') ||
            (Command[StartIndex] == 'w' &&  Command[StartIndex+1] == 'f'))
    {
        (*FDISK).fit[0] = Command[StartIndex];
        (*FDISK).fit[1] = Command[StartIndex+1];
        printf(">>> El valor de FIT es ");
        printf("%s",(*FDISK).fit);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de FIT no es valido.\n");
        return 256;
    }
    return StartIndex;
}

int getFDISKType(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    if(Command[StartIndex] == 'p' || Command[StartIndex] == 'e' || Command[StartIndex] == 'l')
    {
        (*FDISK).type = Command[StartIndex];
        printf(">>> El valor de TYPE es ");
        printf("%c",(*FDISK).type);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de TYPE no es valido.\n");
        return 256;
    }
    return StartIndex;
}

int getFDISKUnit(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    if(Command[StartIndex] == 'm' || Command[StartIndex] == 'k')
    {
        (*FDISK).unit = Command[StartIndex];
        printf(">>> El valor de UNIT es ");
        printf("%c",(*FDISK).unit);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de UNIT no es valido.\n");
        return 256;
    }
    return StartIndex;
}

int getFDISKName(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidFDISK) = 0;
                printf(">>> El valor de NAME tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            if(j < 16)
            {
                (*FDISK).name[j] = Command[i];
                j++;
            }
        }
        printf(">>> El valor de NAME es ");
        printf("%s",(*FDISK).name);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de NAME tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

int getFDISKPath(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*FDISK).path[j] = Command[i];
            j++;
        }
        printf(">>> El valor de PATH es ");
        printf("%s",(*FDISK).path);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de PATH no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

int getFDISKSize(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    int NumberCounter = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
            if(itsNumber(Command[i])==1)
                NumberCounter++;
            else
            {
                NumberCounter = 0;
                i = 256;
            }
    }
    if(NumberCounter > 0)
    {
        char NumberArray[NumberCounter+1];
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NumberCounter; i++)
        {
            NumberArray[j] = Command[i];
            j++;
        }
        NumberArray[NumberCounter] = '\0';
        int TemporalSize = 0;
        sscanf(NumberArray,"%d",&TemporalSize);
        (*FDISK).size = TemporalSize;
        printf(">>> El valor de SIZE es ");
        printf("%d",(*FDISK).size);
        printf(".\n");
    }
    else
    {
        (*ValidFDISK) = 0;
        printf(">>> El valor de SIZE tiene que ser un numero.\n");
        return 256;
    }
    return StartIndex + NumberCounter;
}

void getFDISKParameters(int StartIndex, fdisk *FDISK, int *ValidFDISK)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char SIZEParameter[4] = "size";
            if(compareArray(i,SIZEParameter,4) == 1)
            {
                printf(">> Parametro SIZE reconocido.\n");
                i = getFDISKSize(i+6, &(*FDISK), &(*ValidFDISK));
            }
            char PATHParameter[4] = "path";
            if(compareArray(i,PATHParameter,4) == 1)
            {
                printf(">> Parametro PATH reconocido.\n");
                i = getFDISKPath(i+6, &(*FDISK), &(*ValidFDISK));
            }
            char NAMEParameter[4] = "name";
            if(compareArray(i,NAMEParameter,4) == 1)
            {
                printf(">> Parametro NAME reconocido.\n");
                i = getFDISKName(i+6, &(*FDISK), &(*ValidFDISK));
            }
            char UNITParameter[4] = "unit";
            if(compareArray(i,UNITParameter,4) == 1)
            {
                printf(">> Parametro UNIT reconocido.\n");
                i = getFDISKUnit(i+6, &(*FDISK), &(*ValidFDISK));
            }
            char TYPEParameter[4] = "type";
            if(compareArray(i,TYPEParameter,4) == 1)
            {
                printf(">> Parametro TYPE reconocido.\n");
                i = getFDISKType(i+6, &(*FDISK), &(*ValidFDISK));
            }
            char FITParameter[3] = "fit";
            if(compareArray(i,FITParameter,3) == 1)
            {
                printf(">> Parametro FIT reconocido.\n");
                i = getFDISKFit(i+5, &(*FDISK), &(*ValidFDISK));
            }
            char DELETEParameter[6] = "delete";
            if(compareArray(i,DELETEParameter,6) == 1)
            {
                printf(">> Parametro DELETE reconocido.\n");
                i = getFDISKDelete(i+8, &(*FDISK), &(*ValidFDISK));
            }
            char ADDParameter[3] = "add";
            if(compareArray(i,ADDParameter,3) == 1)
            {
                printf(">> Parametro ADD reconocido.\n");
                i = getFDISKAdd(i+5, &(*FDISK), &(*ValidFDISK));
            }
        }
    }
}

void initializeFDISK(fdisk *FDISK)
{
    (*FDISK).size = 0;
    (*FDISK).unit = 'k';
    memset((*FDISK).path,'\0',96);
    (*FDISK).type = 'p';
    (*FDISK).fit[0] = 'w';
    (*FDISK).fit[1] = 'f';
    memset((*FDISK)._delete,'\0',5);
    memset((*FDISK).name,'\0',16);
    (*FDISK).add = 0;
}

void executeFDISK(int StartIndex)
{
    int ValidFDISK = 1;
    fdisk FDISK;
    initializeFDISK(&FDISK);
    getFDISKParameters(StartIndex,&FDISK,&ValidFDISK);
    if(ValidFDISK == 0){
        printf("> No se puede ejecutar FDISK porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyFDISKData(&FDISK,&ValidFDISK);
        if(ValidFDISK == 0)
        {
            printf("> No se puede ejecutar FDISK por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando FDISK con los parametros definidos.\n");
            runFDISK(&FDISK);
        }
    }
}

void runRMDISK(rmdisk *RMDISK)
{
    FILE *disk;
    if((disk = fopen((*RMDISK).path,"r")))
    {
        fclose(disk);
        if(remove((*RMDISK).path) == 0)
            printf(">> Disco eliminado satisfactoriamente.\n");
        else
        {
            printf(">> No se puede eliminar el disco.\n");
        }
    }
    else
    {
        printf(">> No existe el disco a eliminar.\n");
    }
}

void verifyRMDISKData(rmdisk *RMDISK, int *ValidRMDISK)
{

    int ValidFinalPath = 0;
    for(int i = 0; i < 96; i++)
    {
        if((*RMDISK).path[i] != '\0')
            ValidFinalPath = 1;
    }
    if(ValidFinalPath == 0)
    {
        (*ValidRMDISK) = 0;
        printf(">> No se definio una PATH.\n");
    }
}

int getRMDISKPath(int StartIndex, rmdisk *RMDISK, int *ValidRMDISK)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*RMDISK).path[j] = Command[i];
            j++;
        }
        printf(">>> El valor de PATH es ");
        printf("%s",(*RMDISK).path);
        printf(".\n");
    }
    else
    {
        (*ValidRMDISK) = 0;
        printf(">>> El valor de PATH no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

void getRMDISKParameters(int StartIndex, rmdisk *RMDISK, int *ValidRMDISK)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char PATHParameter[4] = "path";
            if(compareArray(i,PATHParameter,4) == 1)
            {
                printf(">> Parametro PATH reconocido.\n");
                i = getRMDISKPath(i+6, &(*RMDISK), &(*ValidRMDISK));
            }
        }
    }
}

void initializeRMDISK(rmdisk *RMDISK)
{
    memset((*RMDISK).path,'\0',96);
}

void executeRMDISK(int StartIndex)
{
    int ValidRMDISK = 1;
    rmdisk RMDISK;
    initializeRMDISK(&RMDISK);
    getRMDISKParameters(StartIndex,&RMDISK,&ValidRMDISK);
    if(ValidRMDISK == 0){
        printf("> No se puede ejecutar RMDISK porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyRMDISKData(&RMDISK,&ValidRMDISK);
        if(ValidRMDISK == 0)
        {
            printf("> No se puede ejecutar RMDISK por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando RMDISK con los parametros definidos.\n");
            runRMDISK(&RMDISK);
        }
    }
}

void fillMBRData(mbr *MBR, int FinalSize)
{
    (*MBR).mbr_size = FinalSize;
    time(&(*MBR).mbr_creation_date);
    srand(time(0));
    (*MBR).mbr_disk_signature = rand();
    (*MBR).mbr_partition_1.part_status = '0';
    (*MBR).mbr_partition_2.part_status = '0';
    (*MBR).mbr_partition_3.part_status = '0';
    (*MBR).mbr_partition_4.part_status = '0';
}

FILE* createMKDISKFile(mkdisk *MKDISK, int *FinalSize)
{
    int PathLength = 0;
    for(int i = 0; i < 96; i++)
    {
        if((*MKDISK).path[i] == '\0')
            i = 96;
        else
            PathLength++;
    }
    char DiskPath[PathLength];
    for(int i = 0; i < PathLength; i++)
    {
        DiskPath[i] = (*MKDISK).path[i];
    }
    createDirectory(DiskPath,PathLength);
    int NameLength = 0;
    for(int i = 0; i < 32; i++)
    {
        if((*MKDISK).name[i] == '\0')
            i = 32;
        else
            NameLength++;
    }
    char DiskName[NameLength];
    for(int i = 0; i < NameLength; i++)
    {
        DiskName[i] = (*MKDISK).name[i];
    }
    char FinalDiskPath[PathLength+NameLength];
    for(int i = 0; i < PathLength; i++)
    {
        FinalDiskPath[i] = DiskPath[i];
    }
    int j = 0;
    for(int i = PathLength; i < PathLength + NameLength; i++)
    {
        FinalDiskPath[i] = DiskName[j];
        j++;
    }
    FILE *FilePointer;
    FilePointer = fopen(FinalDiskPath,"wb+");
    int DiskSize = 0;
    if((*MKDISK).unit == 'm')
    {
        DiskSize = 1024 * 1024 * (*MKDISK).size;
    }
    else
    {
        DiskSize = 1024 * (*MKDISK).size;
    }
    char Content = '\0';
    for(int i = 0; i < DiskSize; i++)
    {
        fwrite(&Content,sizeof(Content),1,FilePointer);
    }
    (*FinalSize) = DiskSize;
    return FilePointer;
}

void runMKDISK(mkdisk *MKDISK)
{
    FILE *FilePointer;
    int FinalSize = 0;
    FilePointer = createMKDISKFile(&(*MKDISK),&FinalSize);
    mbr MBR;
    fillMBRData(&MBR,FinalSize);
    fseek(FilePointer,0,SEEK_SET);
    fwrite(&MBR,sizeof(mbr),1,FilePointer);
    fclose(FilePointer);
    printf(">> Disco creado correctamente.\n");
}

void verifyMKDISKData(mkdisk *MKDISK, int *ValidMKDISK)
{
    if((*MKDISK).size == 0)
    {
        (*ValidMKDISK) = 0;
        printf(">> No se definio el SIZE.\n");
    }
    if((*ValidMKDISK) != 0)
    {
        int ValidFinalPath = 0;
        for(int i = 0; i < 96; i++)
        {
            if((*MKDISK).path[i] != '\0')
                ValidFinalPath = 1;
        }
        if(ValidFinalPath == 1)
        {
            ValidFinalPath = 0;
            for(int i = 0; i < 32; i++)
            {
                if((*MKDISK).name[i] != '\0')
                    ValidFinalPath = 1;
            }
            if(ValidFinalPath == 0)
            {
                (*ValidMKDISK) = 0;
                printf(">> No se definio un NAME.\n");
            }
        }
        else
        {
            (*ValidMKDISK) = 0;
            printf(">> No se definio una PATH.\n");
        }
    }
}

int getMKDISKUnit(int StartIndex, mkdisk *MKDISK, int *ValidMKDISK)
{
    if(Command[StartIndex] == 'm' || Command[StartIndex] == 'k')
    {
        (*MKDISK).unit = Command[StartIndex];
        printf(">>> El valor de UNIT es ");
        printf("%c",(*MKDISK).unit);
        printf(".\n");
    }
    else
    {
        (*ValidMKDISK) = 0;
        printf(">>> El valor de UNIT no es valido.\n");
        return 256;
    }
    return StartIndex;
}

int getMKDISKName(int StartIndex, mkdisk *MKDISK, int *ValidMKDISK)
{
    int NameLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
        {
            if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                NameLength++;
            else
            {
                (*ValidMKDISK) = 0;
                printf(">>> El valor de NAME tiene que ser un identificador.\n");
                return 256;
            }
        }
    }
    if(NameLength>0)
    {
        char NameArray[NameLength];
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NameLength; i++)
        {
            NameArray[j] = Command[i];
            j++;
        }
        int ValidName = 0;
        for(int i = 0; i < NameLength; i++)
        {
            if(NameArray[i] == '.')
            {
                if(i+3 < NameLength)
                {
                    if(NameArray[i+1] == 'd')
                    {
                        if(NameArray[i+2] == 's')
                        {
                            if(NameArray[i+3] == 'k')
                            {
                                ValidName = 1;
                                i = NameLength;
                            }
                        }
                    }
                }
                else
                {
                    (*ValidMKDISK) = 0;
                    printf(">>> El valor de NAME tiene una extension invalida.\n");
                    return 256;
                }
            }
        }
        if(ValidName == 1)
        {
            j = 0;
            for(int i = StartIndex; i < StartIndex + NameLength; i++)
            {
                (*MKDISK).name[j] = Command[i];
                j++;
            }
            printf(">>> El valor de NAME es ");
            printf("%s",(*MKDISK).name);
            printf(".\n");
        }
        else
        {
            (*ValidMKDISK) = 0;
            printf(">>> El valor de NAME no tiene una extension.\n");
            return 256;
        }
    }
    else
    {
        (*ValidMKDISK) = 0;
        printf(">>> El valor de NAME tiene que ser un identificador.\n");
        return 256;
    }
    return StartIndex + NameLength;
}

int getMKDISKPath(int StartIndex, mkdisk *MKDISK, int *ValidMKDISK)
{
    int QuotesActived = 0;
    if(Command[StartIndex]  == '\"')
    {
        QuotesActived = 1;
        StartIndex++;
    }
    int PathLength = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(QuotesActived == 1)
        {
            if(Command[i] == '\"')
                i = 256;
            else
                PathLength++;
        }
        else
        {
            if(Command[i] == ' ' || Command[i] == '\n')
                i = 256;
            else
                PathLength++;
        }
    }
    if(PathLength > 0)
    {
        int j = 0;
        for(int i = StartIndex; i < StartIndex + PathLength; i++)
        {
            (*MKDISK).path[j] = Command[i];
            j++;
        }
        printf(">>> El valor de PATH es ");
        printf("%s",(*MKDISK).path);
        printf(".\n");
    }
    else
    {
        (*ValidMKDISK) = 0;
        printf(">>> El valor de PATH no es valido.\n");
        return 256;
    }
    return StartIndex + PathLength;
}

int getMKDISKSize(int StartIndex, mkdisk *MKDISK, int *ValidMKDISK)
{
    int NumberCounter = 0;
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == ' ' || Command[i] == '\n')
            i = 256;
        else
            if(itsNumber(Command[i])==1)
                NumberCounter++;
            else
            {
                NumberCounter = 0;
                i = 256;
            }
    }
    if(NumberCounter > 0)
    {
        char NumberArray[NumberCounter+1];
        int j = 0;
        for(int i = StartIndex; i < StartIndex + NumberCounter; i++)
        {
            NumberArray[j] = Command[i];
            j++;
        }
        NumberArray[NumberCounter] = '\0';
        int TemporalSize = 0;
        sscanf(NumberArray,"%d",&TemporalSize);
        (*MKDISK).size = TemporalSize;
        printf(">>> El valor de SIZE es ");
        printf("%d",(*MKDISK).size);
        printf(".\n");
    }
    else
    {
        (*ValidMKDISK) = 0;
        printf(">>> El valor de SIZE tiene que ser un numero.\n");
        return 256;
    }
    return StartIndex + NumberCounter;
}

void getMKDISKParameters(int StartIndex, mkdisk *MKDISK, int *ValidMKDISK)
{
    for(int i = StartIndex; i < 256; i++)
    {
        if(Command[i] == '&')
        {
            i++;
            char SIZEParameter[4] = "size";
            if(compareArray(i,SIZEParameter,4) == 1)
            {
                printf(">> Parametro SIZE reconocido.\n");
                i = getMKDISKSize(i+6, &(*MKDISK), &(*ValidMKDISK));
            }
            char PATHParameter[4] = "path";
            if(compareArray(i,PATHParameter,4) == 1)
            {
                printf(">> Parametro PATH reconocido.\n");
                i = getMKDISKPath(i+6, &(*MKDISK), &(*ValidMKDISK));
            }
            char NAMEParameter[4] = "name";
            if(compareArray(i,NAMEParameter,4) == 1)
            {
                printf(">> Parametro NAME reconocido.\n");
                i = getMKDISKName(i+6, &(*MKDISK), &(*ValidMKDISK));
            }
            char UNITParameter[4] = "unit";
            if(compareArray(i,UNITParameter,4) == 1)
            {
                printf(">> Parametro UNIT reconocido.\n");
                i = getMKDISKUnit(i+6, &(*MKDISK), &(*ValidMKDISK));
            }
        }
    }
}

void initializeMKDISK(mkdisk *MKDISK)
{
    (*MKDISK).size = 0;
    memset((*MKDISK).path,'\0',96);
    memset((*MKDISK).name,'\0',32);
    (*MKDISK).unit = 'm';
}

void executeMKDISK(int StartIndex)
{
    int ValidMKDISK = 1;
    mkdisk MKDISK;
    initializeMKDISK(&MKDISK);
    getMKDISKParameters(StartIndex,&MKDISK,&ValidMKDISK);
    if(ValidMKDISK == 0)
    {
        printf("> No se puede ejecutar MKDISK porque tiene parametros invalidos.\n");
    }
    else
    {
        verifyMKDISKData(&MKDISK,&ValidMKDISK);
        if(ValidMKDISK == 0)
        {
            printf("> No se puede ejecutar MKDISK por falta de parametros.\n");
        }
        else
        {
            printf("> Ejecutando comando MKDISK con los parametros definidos.\n");
            runMKDISK(&MKDISK);
        }
    }
}

void commandToLower()
{
    for(int i = 0; i < 256; i++)
    {
        Command[i] = tolower(Command[i]);
    }
}

void cleanCommand()
{
    for(int i = 0; i < 256; i++)
    {
        Command[i] = '\0';
    }
}

void identifyAction()
{
    int ValidAction = 0;
    for(int i = 0; i < 256; i++)
    {
        char MKDISKAction[6] = "mkdisk";
        if(compareArray(i,MKDISKAction,6) == 1)
        {
            printf("> Comando MKDISK reconocido.\n");
            executeMKDISK(i+6);
            i = 256;
            ValidAction = 1;
        }
        char RMKDISKAction[6] = "rmdisk";
        if(compareArray(i,RMKDISKAction,6) == 1)
        {
            printf("> Comando RMDISK reconocido.\n");
            executeRMDISK(i+6);
            i = 256;
            ValidAction = 1;
        }
        char FDISKAction[5] = "fdisk";
        if(compareArray(i,FDISKAction,5) == 1)
        {
            printf("> Comando FDISK reconocido.\n");
            executeFDISK(i+5);
            i = 256;
            ValidAction = 1;
        }
        char MOUNTAction[5] = "mount";
        if(compareArray(i,MOUNTAction,5) == 1)
        {
            printf("> Comando MOUNT reconocido.\n");
            executeMOUNT(i+5);
            i = 256;
            ValidAction = 1;
        }
        char UNMOUNTAction[7] = "unmount";
        if(compareArray(i,UNMOUNTAction,7) == 1)
        {
            printf("> Comando UNMOUNT reconocido.\n");
            executeUNMOUNT(i+7);
            i = 256;
            ValidAction = 1;
        }
        char EXITAction[4] = "exit";
        if(compareArray(i,EXITAction,4) == 1)
        {
            printf("> Comando EXIT reconocido.\n");
            exit(0);
        }
        char EXECAction[4] = "exec";
        if(compareArray(i,EXECAction,4) == 1)
        {
            printf("> Comando EXEC reconocido.\n");
            i = i + 5;
            char ExecPath[96];
            memset(ExecPath,'\0',96);
            int k = 0;
            for(int j = i; j < 256; j++)
            {
                if(Command[j] == '\n')
                {
                    j = 256;
                }
                else
                {
                    ExecPath[k] = Command[j];
                    k++;
                }
            }
            FILE *Script;
            Script = fopen(ExecPath,"r");
            if(Script == NULL)
            {
                printf(">> El script no existe.\n");
            }
            else
            {
                printf("%s\n",ExecPath);
                while(fgets(Command,128,Script) != NULL)
                {
                    char AnotherLine = 0;
                    for(int j = 0; j < 127;j++)
                    {
                        if(Command[j] == '\\' && Command[j+1] == '^')
                            AnotherLine = j;
                    }
                    if(AnotherLine != 0)
                    {
                        char SecondCommand[128];
                        memset(SecondCommand, '\0',128);
                        fgets(SecondCommand,128,Script);
                        int k = 0;
                        for(int j = AnotherLine+3; j < 256; j++)
                        {
                            if(k < 128)
                            {
                                Command[j] = SecondCommand[k];
                                k++;
                            }
                        }
                    }
                    printf("\n");
                    commandToLower();
                    printf("%s",Command);
                    identifyAction();
                    cleanCommand();
                }
                fclose(Script);
            }
            i = 256;
            ValidAction = 1;
        }
        char PAUSEAction[5] = "pause";
        if(compareArray(i,PAUSEAction,5) == 1)
        {
            printf("> Comando PAUSE reconocido.\n");
            char Pause[2];
            printf(">> Presione cualquier tecla para continuar.\n");
            fgets(Pause,2,stdin);
            i = 256;
            ValidAction = 1;
        }
        char MKFSAction[4] = "mkfs";
        if(compareArray(i,MKFSAction,4) == 1)
        {
            printf("> Comando MKFS reconocido.\n");
            executeMKFS(i+4);
            i = 256;
            ValidAction = 1;
        }
        char MKDIRAction[5] = "mkdir";
        if(compareArray(i,MKDIRAction,5) == 1)
        {
            printf("> Comando MKDIR reconocido.\n");
            executeMKDIR(i+5);
            i = 256;
            ValidAction = 1;
        }
        char MKFILEAction[6] = "mkfile";
        if(compareArray(i,MKFILEAction,6) == 1)
        {
            printf("> Comando MKFILE reconocido.\n");
            executeMKFILE(i+6);
            i = 256;
            ValidAction = 1;
        }
        char REPAction[3] = "rep";
        if(compareArray(i,REPAction,3) == 1)
        {
            printf("> Comando REP reconocido.\n");
            executeREP(i+3);
            i = 256;
            ValidAction = 1;
        }
        char LOSSAction[4] = "loss";
        if(compareArray(i,LOSSAction,4) == 1)
        {
            printf("> Comando LOSS reconocido.\n");
            int StartIndex = i + 10;
            int NameLength = 0;
            char Id[6];
            memset(Id,'\0',6);
            for(int i = StartIndex; i < 256; i++)
            {
                if(Command[i] == ' ' || Command[i] == '\n')
                    i = 256;
                else
                {
                    if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                        NameLength++;
                    else
                    {
                        printf(">>> El valor de ID tiene que ser un identificador.\n");
                    }
                }
            }
            if(NameLength>0)
            {
                int j = 0;
                for(int i = StartIndex; i < StartIndex + NameLength; i++)
                {
                    if(j < 16)
                    {
                        Id[j] = Command[i];
                        j++;
                    }
                }
                printf(">>> El valor de ID es ");
                printf("%s",Id);
                printf(".\n");
                int MountedIndex = -1;
                for(int i = 0; i < 256; i++)
                {
                    if(MountedPartitions[i].active == '1' && MountedPartitions[i].letter == Id[2]
                            && MountedPartitions[i].number == (Id[3] - 48))
                    {
                        MountedIndex = i;
                        i = 256;
                    }
                }
                if(MountedIndex != -1)
                {
                    FILE *disk;
                    if((disk = fopen(MountedPartitions[MountedIndex].path,"rb+")))
                    {
                        mbr MBR;
                        fseek(disk,0,SEEK_SET);
                        fread(&MBR,sizeof(mbr),1,disk);
                        int NameMatch[4];
                        NameMatch[0] = 0;
                        NameMatch[1] = 0;
                        NameMatch[2] = 0;
                        NameMatch[3] = 0;
                        for(int i = 0; i < 16; i++)
                        {
                            if(MBR.mbr_partition_1.part_status == '1'
                                    && MBR.mbr_partition_1.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[0]++;
                            if(MBR.mbr_partition_2.part_status == '1'
                                    && MBR.mbr_partition_2.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[1]++;
                            if(MBR.mbr_partition_3.part_status == '1'
                                    && MBR.mbr_partition_3.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[2]++;
                            if(MBR.mbr_partition_4.part_status == '1'
                                    && MBR.mbr_partition_4.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[3]++;
                        }
                        int PartitionStartIndex = 0;
                        int PartitionTotalSize = 0;
                        if(NameMatch[0] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_1.part_start;
                            PartitionTotalSize = MBR.mbr_partition_1.part_size;
                        }
                        if(NameMatch[1] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_2.part_start;
                            PartitionTotalSize = MBR.mbr_partition_2.part_size;
                        }
                        if(NameMatch[2] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_3.part_start;
                            PartitionTotalSize = MBR.mbr_partition_3.part_size;
                        }
                        if(NameMatch[3] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_4.part_start;
                            PartitionTotalSize = MBR.mbr_partition_4.part_size;
                        }
                        if(NameMatch[0] == 16 || NameMatch[1] == 16 || NameMatch[2] == 16 || NameMatch[3] == 16)
                        {
                            fseek(disk,PartitionStartIndex,SEEK_SET);
                            sb SB;
                            initializeSB(&SB);
                            fread(&SB,sizeof(sb),1,disk);
                            char EraseContent = '\0';
                            fseek(disk,PartitionStartIndex,SEEK_SET);
                            for(int i = PartitionStartIndex; i < SB.sb_p_log - 1; i++)
                            {
                                fwrite(&EraseContent,sizeof(char),1,disk);
                            }
                            printf(">> Simulacion de perdida exitosa.\n");
                        }
                        else
                        {
                            printf(">> No se encontro la particion en el disco.\n");
                        }
                        fclose(disk);
                    }
                    else
                    {
                        printf(">> El disco no existe.\n");
                    }
                }
                else
                {
                    printf("La particion no se encuentra montada.\n");
                }
            }
            else
            {
                printf(">>> El valor de ID tiene que ser un identificador.\n");
            }
            i = 256;
            ValidAction = 1;
        }
        char RECOVERYAction[8] = "recovery";
        if(compareArray(i,RECOVERYAction,8) == 1)
        {
            printf("> Comando RECOVERY reconocido.\n");
            int StartIndex = i + 14;
            int NameLength = 0;
            char Id[6];
            memset(Id,'\0',6);
            for(int i = StartIndex; i < 256; i++)
            {
                if(Command[i] == ' ' || Command[i] == '\n')
                    i = 256;
                else
                {
                    if(itsIdentifier(Command[i]) == 1 || Command[i] == '.')
                        NameLength++;
                    else
                    {
                        printf(">>> El valor de ID tiene que ser un identificador.\n");
                    }
                }
            }
            if(NameLength>0)
            {
                int j = 0;
                for(int i = StartIndex; i < StartIndex + NameLength; i++)
                {
                    if(j < 16)
                    {
                        Id[j] = Command[i];
                        j++;
                    }
                }
                printf(">>> El valor de ID es ");
                printf("%s",Id);
                printf(".\n");
                int MountedIndex = -1;
                for(int i = 0; i < 256; i++)
                {
                    if(MountedPartitions[i].active == '1' && MountedPartitions[i].letter == Id[2]
                            && MountedPartitions[i].number == (Id[3] - 48))
                    {
                        MountedIndex = i;
                        i = 256;
                    }
                }
                if(MountedIndex != -1)
                {
                    FILE *disk;
                    if((disk = fopen(MountedPartitions[MountedIndex].path,"rb+")))
                    {
                        mbr MBR;
                        fseek(disk,0,SEEK_SET);
                        fread(&MBR,sizeof(mbr),1,disk);
                        int NameMatch[4];
                        NameMatch[0] = 0;
                        NameMatch[1] = 0;
                        NameMatch[2] = 0;
                        NameMatch[3] = 0;
                        for(int i = 0; i < 16; i++)
                        {
                            if(MBR.mbr_partition_1.part_status == '1'
                                    && MBR.mbr_partition_1.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[0]++;
                            if(MBR.mbr_partition_2.part_status == '1'
                                    && MBR.mbr_partition_2.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[1]++;
                            if(MBR.mbr_partition_3.part_status == '1'
                                    && MBR.mbr_partition_3.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[2]++;
                            if(MBR.mbr_partition_4.part_status == '1'
                                    && MBR.mbr_partition_4.part_name[i] == MountedPartitions[MountedIndex].name[i])
                                NameMatch[3]++;
                        }
                        int PartitionStartIndex = 0;
                        int PartitionTotalSize = 0;
                        if(NameMatch[0] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_1.part_start;
                            PartitionTotalSize = MBR.mbr_partition_1.part_size;
                        }
                        if(NameMatch[1] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_2.part_start;
                            PartitionTotalSize = MBR.mbr_partition_2.part_size;
                        }
                        if(NameMatch[2] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_3.part_start;
                            PartitionTotalSize = MBR.mbr_partition_3.part_size;
                        }
                        if(NameMatch[3] == 16)
                        {
                            PartitionStartIndex = MBR.mbr_partition_4.part_start;
                            PartitionTotalSize = MBR.mbr_partition_4.part_size;
                        }
                        if(NameMatch[0] == 16 || NameMatch[1] == 16 || NameMatch[2] == 16 || NameMatch[3] == 16)
                        {
                            printf(">> Buscando copia de recuperacion...\n");
                            sb SB;
                            initializeSB(&SB);
                            for(int i = PartitionStartIndex; i < PartitionStartIndex + PartitionTotalSize; i++)
                            {
                                fseek(disk,i,SEEK_SET);
                                fread(&SB,sizeof(sb),1,disk);
                                if(SB.sb_magic_number == 201504468)
                                {
                                    i = PartitionStartIndex + PartitionTotalSize;
                                    fseek(disk,SB.sb_p_log,SEEK_SET);

                                    printf(">> SB de respaldo encontrado.\n>> Intentando recuperar sistema.\n");
                                }
                            }
                            int TotalStructures = getTotalStructures(PartitionTotalSize);
                            char BitmapContent = '0';
                            time_t Time = time(0);
                            struct tm *tlocal = localtime(&Time);
                            char TemporalTime[24];
                            strftime(TemporalTime,24,"%d/%m/%y %H:%M:%S",tlocal);
                            fseek(disk,SB.sb_p_tree_directory_bitmap,SEEK_SET);
                            for(int i = 0; i < TotalStructures; i++)
                            {
                                fwrite(&BitmapContent,sizeof(char),1,disk);
                            }
                            fseek(disk,SB.sb_p_detail_directory_bitmap,SEEK_SET);
                            for(int i = 0; i < TotalStructures; i++)
                            {
                                fwrite(&BitmapContent,sizeof(char),1,disk);
                            }
                            fseek(disk,SB.sb_p_inode_bitmap,SEEK_SET);
                            for(int i = 0; i < TotalStructures; i++)
                            {
                                fwrite(&BitmapContent,sizeof(char),1,disk);
                            }
                            fseek(disk,SB.sb_p_block_bitmap,SEEK_SET);
                            for(int i = 0; i < TotalStructures; i++)
                            {
                                fwrite(&BitmapContent,sizeof(char),1,disk);
                            }
                            tvd TVD;
                            initializeTVD(&TVD);
                            strcpy(TVD.tvd_creation_date,TemporalTime);
                            TVD.tvd_directory_name[0] = '/';
                            TVD.tvd_p_detail_directory = SB.sb_p_detail_directory;
                            TVD.tvd_p_tvd = -1;
                            TVD.tvd_proprietary = 1;
                            fseek(disk,SB.sb_p_tree_directory_bitmap,SEEK_SET);
                            BitmapContent = '1';
                            fwrite(&BitmapContent,sizeof(char),1,disk);
                            fseek(disk,SB.sb_p_tree_directory,SEEK_SET);
                            fwrite(&TVD,sizeof(tvd),1,disk);
                            SB.sb_virtual_tree_count = SB.sb_virtual_tree_count + 1;
                            SB.sb_virtual_tree_free = SB.sb_virtual_tree_free - 1;
                            SB.sb_ffb_tree_directory = 1;
                            dd DD;
                            initializeDD(&DD);
                            strcpy(DD.files[0].dd_file_name,"users.txt");
                            DD.files[0].dd_file_p_inode = SB.sb_p_inode;
                            strcpy(DD.files[0].dd_file_creation_date,TemporalTime);
                            strcpy(DD.files[0].dd_file_modified_date,TemporalTime);
                            DD.dd_p_dd = -1;
                            fseek(disk,SB.sb_p_detail_directory_bitmap,SEEK_SET);
                            fwrite(&BitmapContent,sizeof(char),1,disk);
                            fseek(disk,SB.sb_p_detail_directory,SEEK_SET);
                            fwrite(&DD,sizeof(dd),1,disk);
                            SB.sb_detail_directory_count = SB.sb_detail_directory_count + 1;
                            SB.sb_detail_directory_free = SB.sb_detail_directory_free - 1;
                            SB.sb_ffb_detail_directory = 1;
                            inode INODE;
                            initializeINODE(&INODE);
                            INODE.i_count = 0;
                            INODE.i_file_size = 2*sizeof(block);
                            INODE.i_block_count = 2;
                            INODE.i_p_block[0] = SB.sb_p_block;
                            INODE.i_p_block[1] = SB.sb_p_block + sizeof(block);
                            INODE.i_p_i = -1;
                            INODE.i_proprietary = 1;
                            fseek(disk,SB.sb_p_inode_bitmap,SEEK_SET);
                            fwrite(&BitmapContent,sizeof(char),1,disk);
                            fseek(disk,SB.sb_p_inode,SEEK_SET);
                            fwrite(&INODE,sizeof(inode),1,disk);
                            SB.sb_inode_count = SB.sb_inode_count + 1;
                            SB.sb_inode_free =  SB.sb_inode_free - 1;
                            SB.sb_ffb_inode = 1;
                            block BLOCK1;
                            initializeBLOCK(&BLOCK1);
                            strcpy(BLOCK1.block_data,"1,G,root \n 1,U,");
                            fseek(disk,SB.sb_p_block_bitmap,SEEK_SET);
                            fwrite(&BitmapContent,sizeof(char),1,disk);
                            fseek(disk,SB.sb_p_block,SEEK_SET);
                            fwrite(&BLOCK1,sizeof(block),1,disk);
                            SB.sb_block_count = SB.sb_block_count + 1;
                            SB.sb_block_free =  SB.sb_block_free - 1;
                            SB.sb_ffb_block = 1;
                            block BLOCK2;
                            initializeBLOCK(&BLOCK2);
                            strcpy(BLOCK2.block_data,"root,root,201504468 \n");
                            fseek(disk,SB.sb_p_block_bitmap + sizeof(char),SEEK_SET);
                            fwrite(&BitmapContent,sizeof(char),1,disk);
                            fseek(disk,SB.sb_p_block + sizeof(block),SEEK_SET);
                            fwrite(&BLOCK2,sizeof(block),1,disk);
                            SB.sb_block_count = SB.sb_block_count + 1;
                            SB.sb_block_free =  SB.sb_block_free - 1;
                            SB.sb_ffb_block = 2;
                            fseek(disk,PartitionStartIndex,SEEK_SET);
                            fwrite(&SB,sizeof(sb),1,disk);
                            fseek(disk,SB.sb_p_log,SEEK_SET);
                            int TotalLogs;
                            fread(&TotalLogs,sizeof(int),1,disk);
                            for(int i = 0; i < TotalLogs; i++)
                            {
                                log LOG;
                                initializeLOG(&LOG);
                                int LogIndex = SB.sb_p_log + sizeof(int) + i * sizeof(log);
                                fseek(disk,LogIndex,SEEK_SET);
                                fread(&LOG,sizeof(log),1,disk);
                                strcpy(Command,LOG.log_operation);
                                printf("\n");
                                commandToLower();
                                printf("%s",Command);
                                identifyAction();
                                cleanCommand();
                            }
                            printf(">> Recovery Exitoso.\n");
                        }
                        else
                        {
                            printf(">> No se encontro la particion en el disco.\n");
                        }
                        fclose(disk);
                    }
                    else
                    {
                        printf(">> El disco no existe.\n");
                    }
                }
                else
                {
                    printf("La particion no se encuentra montada.\n");
                }
            }
            else
            {
                printf(">>> El valor de ID tiene que ser un identificador.\n");
            }
            i = 256;
            ValidAction = 1;
        }
    }
    if(ValidAction == 0)
    {
        printf("> Comando no valido.\n");
    }
}

void readCommand()
{
    while(1)
    {
        printf("\n");
        cleanCommand();
        fgets(Command,256,stdin);
        commandToLower();
        identifyAction();
    }
}

void initializeMountedPartitions()
{
    for(int i = 0; i < 256; i++)
    {
        MountedPartitions[i].active = '0';
        MountedPartitions[i].letter = '\0';
        MountedPartitions[i].number = 0;
        memset(MountedPartitions[i].path,'\0',96);
        memset(MountedPartitions[i].name,'\0',16);
    }
}

int main()
{
    printf("Proyecto No. 1 - Guillermo Peitzner - 201504468.\n");
    initializeMountedPartitions();
    readCommand();
    return 0;
}
