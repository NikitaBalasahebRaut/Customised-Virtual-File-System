#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<iostream>

#define MAXFILES 100
#define FILESIZE 1024

#define READ 1
#define WRITE 2

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

struct SuperBlock
{
	int TotalInodes;
	int FreeInode;
}Obj_Super;

struct inode
{
	char File_name[50];
	int Inode_Number;
	int File_Size;
	int File_Type;
	int ActualFileSize;
	int Link_Count;
	int Reference_Cout;
	int Permission;
	char *Data;
	struct inode *next;
};

typedef struct inode INODE;
typedef struct inode * PINODE;
typedef struct inode ** PPINODE;

struct FileTable
{
	int ReadOffset;
	int WriteOffset;
	int Count;
	PINODE iptr;
	int Mode;
};

typedef FileTable FILETABLE;
typedef FileTable * PFILETABLE;

struct UFDT
{
	PFILETABLE ufdt[MAXFILES];
}UFTDObj;


PINODE Head = NULL;  //Global pointer of inode

//it used to check wether file is present or not
bool ChekFile(char *name)
{
	PINODE temp = Head;
	while(temp != NULL)
	{
		if(temp->File_Type == 0) //file_Type == 0 means file is deleted
		{
			if(strcmp(temp->File_name,name) == 0)
			{
				break;     //name find zal ki break
			}
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		 return false;
	}
	else
	{
		return true;
	}
}
	PINODE Get_Inode(char *name)
	{
		PINODE temp = Head;
		int i = 0;
		
		if(name == NULL)
		    return NULL;
		
		while(temp != NULL)
		{
			if(strcmp(name,temp->File_name) == 0)
				break;
			temp = temp->next;
		}
		return temp;
	}
void CreateUFDT()
{
	int i = 0;
	
	for(i = 0; i< MAXFILES; i++)
	{
		UFTDObj.ufdt[i] = NULL;
	}
}
void CreateDILB()  //create linked list of inodes
{
	int i = 1;
	PINODE newn = NULL;
	PINODE temp = Head;
	
	while(i <= MAXFILES)   //loop iterate 100 times
	{
		newn = (PINODE)malloc(sizeof(INODE));
		
		newn->Inode_Number = i;
		newn->File_Size = FILESIZE;
		newn->File_Type = 0;
		newn->ActualFileSize = 0;
		newn->Link_Count = 0;
		newn->Reference_Cout = 0;
		newn->Data = NULL;
		newn->next = NULL;
		
		if(Head == NULL)
		{
			Head = newn;
			temp = Head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}
		i++;
	}
	printf("DILB Created Successfully !!\n");
}
void CreateSuperBlock()
{
   Obj_Super.TotalInodes = MAXFILES;
   Obj_Super.FreeInode = MAXFILES;

   printf("Super Block Created Successfully \n");
}   
		
void SetEnvoirnment()
{
	CreateDILB();
	CreateSuperBlock();
	CreateUFDT();
	printf("Envoirnment for the virtual file System is set ...\n");
}

void DeleteFile(char *name)
{
   bool bret = false;

   if(name == NULL)
   {
        return;
   }
   
   bret = ChekFile(name);
   if(bret == false)
   {
      printf("There is no such file \n");
      return;
   }	

   //Search UFDT entry
   int i = 0;
   for(i = 0; i < MAXFILES; i++)
   {
      	 if(strcmp(UFTDObj.ufdt[i]->iptr->File_name,name) == 0)
		 {
                 break;
		 }
   }
   
    strcpy(UFTDObj.ufdt[i]->iptr->File_name,"");
	UFTDObj.ufdt[i]->iptr->File_Type = 0;
	UFTDObj.ufdt[i]->iptr->ActualFileSize = 0;
	UFTDObj.ufdt[i]->iptr->Link_Count = 0;
	UFTDObj.ufdt[i]->iptr->Reference_Cout = 0;

     //Free the memory of file
      free( UFTDObj.ufdt[i]->iptr->Data);
      
      free(UFTDObj.ufdt[i]);

      UFTDObj.ufdt[i] = NULL;

      Obj_Super.FreeInode++;
}	  
int CreateFile(char *name,int Permission)
{
	int i = 0;
	bool bret = false;
	
	if((name == NULL)||(Permission == 0))
	{
		printf("not possible to create file pls enter valid info\n");
		return -1;
	}
	
	bret = ChekFile(name);
	if(bret == true)
	{
		printf("File is already present \n");
		return -1;
	}
	
	if(Obj_Super.FreeInode == 0)
	{
		printf("There is no inode to create the file \n");
		return -1;
	}
	
	//search for empty entry from UFDT
	for(i = 0; i< MAXFILES; i++)
	{
		if(UFTDObj.ufdt[i] == NULL)
		{
			break;
		}
	}
	
	if(i == MAXFILES)
	{
		printf("Unable to get entry in UFDT \n");
		return -1;
	}
	
	//Allocate memory for file table
	UFTDObj.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));
	
	//Initilize the file table
	UFTDObj.ufdt[i]->ReadOffset = 0;
	UFTDObj.ufdt[i]->WriteOffset = 0;
	UFTDObj.ufdt[i]->Mode = Permission;
	UFTDObj.ufdt[i]->Count = 1;
	
	//Search empty inode
	PINODE temp = Head;
	while(temp != NULL)
	{
		if(temp->File_Type == 0)
		{
			break;
		}
		temp = temp->next;
	}
	
	UFTDObj.ufdt[i]->iptr = temp;
	strcpy(UFTDObj.ufdt[i]->iptr->File_name,name);
	UFTDObj.ufdt[i]->iptr->File_Type = REGULAR;
	UFTDObj.ufdt[i]->iptr->ActualFileSize = 0;
	UFTDObj.ufdt[i]->iptr->Link_Count = 1;
	UFTDObj.ufdt[i]->iptr->File_Size = FILESIZE;
	UFTDObj.ufdt[i]->iptr->Reference_Cout = 1;
	
	//Allocate memory for files data
	UFTDObj.ufdt[i]->iptr->Data = (char *)malloc(sizeof(FILESIZE));
	
	Obj_Super.FreeInode--;
	return i;	
}


int ReadFile(int fd,char *arr,int iSize)
{
	int read_Size = 0;
	
	if(UFTDObj.ufdt[fd] == NULL)
		return -1;
	
	if(UFTDObj.ufdt[fd]->Mode != READ && UFTDObj.ufdt[fd]->Mode != READ+WRITE);
	   return -2;
	   
	   if(UFTDObj.ufdt[fd]->iptr->Permission != READ && UFTDObj.ufdt[fd]->iptr->Permission != READ+WRITE)
	     return -2;
       
	   if(UFTDObj.ufdt[fd]->ReadOffset == UFTDObj.ufdt[fd]->iptr->ActualFileSize)
		  return -3;
	  
     if(UFTDObj.ufdt[fd]->iptr->File_Type != REGULAR)
        return -4;

     read_Size = (UFTDObj.ufdt[fd]->iptr->ActualFileSize) - (UFTDObj.ufdt[fd]->ReadOffset);
     if(read_Size < iSize)
	 {
        strncpy(arr,(UFTDObj.ufdt[fd]->iptr->Data)+(UFTDObj.ufdt[fd]->ReadOffset),read_Size);		 
              
		UFTDObj.ufdt[fd]->ReadOffset = UFTDObj.ufdt[fd]->ReadOffset + read_Size;
	 }
	 else
	 {
		 strncpy(arr,(UFTDObj.ufdt[fd]->iptr->Data) + (UFTDObj.ufdt[fd]->ReadOffset),iSize);
		 
		   (UFTDObj.ufdt[fd]->ReadOffset) = (UFTDObj.ufdt[fd]->ReadOffset) + iSize;
	 }
	 return iSize; 
}	 

int LseekFile(int fd,int size,int from)
{
	if((fd < 0) || (from > 2))
		return -1;
	
	if(UFTDObj.ufdt[fd] == NULL)
		return -1;
	
	if((UFTDObj.ufdt[fd]->Mode == READ) || (UFTDObj.ufdt[fd]->Mode == READ+WRITE))
	{
         if(from == CURRENT)
		 {
            if(((UFTDObj.ufdt[fd]->ReadOffset) + size) > UFTDObj.ufdt[fd]->iptr->ActualFileSize)
              return -1;
            if(((UFTDObj.ufdt[fd]->ReadOffset) + size) < 0)
              return -1;
              (UFTDObj.ufdt[fd]->ReadOffset) = (UFTDObj.ufdt[fd]->ReadOffset)+size;
		 }
         else if(from == START)
		 {
           if(size > (UFTDObj.ufdt[fd]->iptr->ActualFileSize))  
              return -1;
            if(size < 0)
               return -1;
            (UFTDObj.ufdt[fd]->ReadOffset) = size;
		 }
         else if(from == END)
		 {
            if((UFTDObj.ufdt[fd]->iptr->ActualFileSize) + size > FILESIZE);
              return -1;
            if(((UFTDObj.ufdt[fd]->ReadOffset) + size) < 0)
              return -1;
		  (UFTDObj.ufdt[fd]->ReadOffset) = (UFTDObj.ufdt[fd]->iptr->ActualFileSize)+size;
		 }
	}
    else if(UFTDObj.ufdt[fd]->Mode == WRITE)
	{
		if(from == CURRENT)
		{
			if(((UFTDObj.ufdt[fd]->WriteOffset) + size) > FILESIZE)
				return -1;
			if(((UFTDObj.ufdt[fd]->WriteOffset) + size) < 0)
				return -1;
			if(((UFTDObj.ufdt[fd]->WriteOffset) + size) > (UFTDObj.ufdt[fd]->iptr->ActualFileSize))
				
		        (UFTDObj.ufdt[fd]->iptr->ActualFileSize) = (UFTDObj.ufdt[fd]->WriteOffset) + size;
                (UFTDObj.ufdt[fd]->WriteOffset) = (UFTDObj.ufdt[fd]->WriteOffset) + size;
		}
		else if(from == START)
		{
			if(size > FILESIZE)
				return -1;
			if(size < 0)
				return -1;
			if(size > (UFTDObj.ufdt[fd]->iptr->ActualFileSize))
				(UFTDObj.ufdt[fd]->iptr->ActualFileSize) = size;
			    (UFTDObj.ufdt[fd]->WriteOffset) = size;
		}
		else if(from == END)
		{
			 if((UFTDObj.ufdt[fd]->iptr->ActualFileSize) + size > FILESIZE)
				 return -1;
			 if(((UFTDObj.ufdt[fd]->WriteOffset) + size) < 0)
				 return -1;
			 (UFTDObj.ufdt[fd]->WriteOffset) = (UFTDObj.ufdt[fd]->iptr->ActualFileSize) + size;
		}
	}
	return 0;
}
			
int OpenFile(char *name,int mode)
{
	int i = 0;
	PINODE temp = NULL;
	
	if(name == NULL || mode <= 0)
		 return -1;
	 
	 temp = Get_Inode(name);
	 if(temp == NULL)
		 return -2;
	 
	 if(temp->Permission < mode)
		 return -3;
	 
	 while(i < FILESIZE)
	 {
		 if(UFTDObj.ufdt[i] == NULL)
			 break;
		 i++;
	 }
	 
	 UFTDObj.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));
	 if(UFTDObj.ufdt[i] == NULL)
		 return -1;
	 
	 UFTDObj.ufdt[i]->Count = 1;
	 UFTDObj.ufdt[i]->Mode = mode;
	 
	 if(mode == READ + WRITE)
	 {
		 UFTDObj.ufdt[i]->ReadOffset = 0;
		 UFTDObj.ufdt[i]->WriteOffset = 0;
	 }
	 else if(mode == READ)
	 {
		 UFTDObj.ufdt[i]->ReadOffset = 0;
	 }
	 else if(mode == WRITE)
	 {
		 UFTDObj.ufdt[i]->WriteOffset = 0;
	 }
	 UFTDObj.ufdt[i]->iptr = temp;
	 (UFTDObj.ufdt[i]->iptr->Reference_Cout)++;
	 
	 return i;
}
	 
void LS()
{
	PINODE temp = Head;
	
	if(Obj_Super.FreeInode == MAXFILES)
	{
		printf("Error : There is no files \n");
		return;
	}
	while(temp != NULL)
	{
		if(temp -> File_Type != 0)
		{
			printf("%s\n",temp->File_name);
		}
		temp = temp->next;
	}
}

int WriteFile(int fd,char *arr,int size)
{
	 if(UFTDObj.ufdt[fd] == NULL)
	 {
		 printf("Invalid file descriptor \n");
		 return -1;
	 }
	 
	 if(UFTDObj.ufdt[fd]->Mode == READ)
	 {
		 printf("There is no write permission\n");
		 return -1;
	 }
	 
	 //Data gets copied into the buffer
    strncpy(((UFTDObj.ufdt[fd]->iptr->Data)+(UFTDObj.ufdt[fd]->WriteOffset)),arr,size);
    
    UFTDObj.ufdt[fd]->WriteOffset = UFTDObj.ufdt[fd]->WriteOffset + size;
	 
	 return size;
	 
}
int GetFDFromName(char *name)
{
   int i = 0;

   while(i < FILESIZE)
   {
      if(UFTDObj.ufdt[i] != NULL)
        if(strcmp((UFTDObj.ufdt[i]->iptr->File_name),name) == 0)
            break;
			i++;
   }
   if(i == FILESIZE)
   {
	   return -1;
   }
   else
   {
	   return i;
   }
}
int stat_file(char *name)
{
  PINODE temp = Head;
  int i = 0;

  if(name == NULL)
  return -1;
	
  while(temp != NULL)
  {
    if(strcmp(name,temp->File_name) == 0)
         break;
     temp = temp->next;
  }
  
  if(temp == NULL)
  return -2;

  printf("\n------statistic Information about file-----\n");
  printf("File name :%s\n",temp->File_name);
  printf("Inode Number %d\n",temp->Inode_Number);
  printf("File size: %d\n",temp-> File_Size);
  printf("ActualFileSize: %d\n",temp->ActualFileSize);
  printf("Link Count: %d\n",temp->Link_Count);
  printf(" Reference_Cout :%d\n",temp->Reference_Cout);
  
  if(temp->Permission == 1)
	  printf("File Permission : Read Only\n");
  else if(temp->Permission == 2)
	  printf("File Permission : write\n");
  else if(temp->Permission == 3)
	  printf("File Permission : Read and write \n");
  printf("--------------------------------\n");
  return 0;
}
 
int fstat_file(int fd)
{
  PINODE temp = Head;
  int i = 0;

  if(fd < 0)
  return -1;
	
  if(UFTDObj.ufdt[fd] == NULL)
  return -2;

  temp = UFTDObj.ufdt[fd]->iptr;
  
  printf("\n------statistic Information about file-----\n");
  printf("File name :%s\n",temp->File_name);
  printf("Inode Number %d\n",temp->Inode_Number);
  printf("File size: %d\n",temp-> File_Size);
  printf("ActualFileSize: %d\n",temp->ActualFileSize);
  printf("Link Count: %d\n",temp->Link_Count);
  printf(" Reference_Cout :%d\n",temp->Reference_Cout);
  
  if(temp->Permission == 1)
	  printf("File Permission : Read Only\n");
  else if(temp->Permission == 2)
	  printf("File Permission : write\n");
  else if(temp->Permission == 3)
	  printf("File Permission : Read and write \n");
  printf("--------------------------------\n");
  return 0;
}
   
void CloseFileByName(int fd)
{
  UFTDObj.ufdt[fd]->ReadOffset = 0;
  UFTDObj.ufdt[fd]->WriteOffset = 0;
  (UFTDObj.ufdt[fd]->iptr->Reference_Cout)--;
}
  
void CloseAllFile()
{
	int i = 0;
	while(i < FILESIZE)
	{
	  if(UFTDObj.ufdt[i] != NULL)
	  {
		 UFTDObj.ufdt[i]->ReadOffset = 0;
         UFTDObj.ufdt[i]->WriteOffset = 0;
         (UFTDObj.ufdt[i]->iptr->Reference_Cout)--;
          break;
	  }
	  i++;
	}
}

int CloseFileByName(char *name)
{
	int i = 0;
	i = GetFDFromName(name);
	
	if(i == -1)
		return -1;
  UFTDObj.ufdt[i]->ReadOffset = 0;
  UFTDObj.ufdt[i]->WriteOffset = 0;
  (UFTDObj.ufdt[i]->iptr->Reference_Cout)--;
  
  return 0;
}
int rm_File(char *name)
{
	int fd = 0;
	
	fd = GetFDFromName(name);
	
	if(fd == -1)
		return -1;
	
	(UFTDObj.ufdt[fd]->iptr->Link_Count)--;
	
	if(UFTDObj.ufdt[fd]->iptr->Link_Count == 0)
	{
		UFTDObj.ufdt[fd]->iptr->File_Type = 0;
		free( UFTDObj.ufdt[fd]);
		printf("File Deleted successfully");
	}
	UFTDObj.ufdt[fd] = NULL;
	(Obj_Super.FreeInode)++;
	return 0;
}
	
int truncate_File(char *name)
{
  int fd = GetFDFromName(name);
  if(fd == -1)
      return -1;

  memset(UFTDObj.ufdt[fd]->iptr->Data,0,1024);
  UFTDObj.ufdt[fd]->ReadOffset = 0;
  UFTDObj.ufdt[fd]->WriteOffset = 0;
  UFTDObj.ufdt[fd]->iptr->ActualFileSize = 0;
  return 0;
}  
void DisplayHelp()
{
	printf("-------------------------------------\n");
	printf("open  : It is used to open the existing file\n");
	printf("close : it is used to close the opened file \n");
	printf("read  : It is used to read the content of file \n");
	printf("write : It is used to write the data into file \n");
	printf("lseek : It is used to change the offset of file\n");
	printf("stat  : It is used to display the information of file \n");
	printf("fstat : It is used to display the information of opend file \n");
	printf("create : It is used to create new regular file\n");
	printf("rm :  It is used to delete regular file \n");
	printf("ls : It is used to display all names of files \n");
	printf("cls : It is used to clear console \n");
	printf("closeall : It is used to close all opened file \n");
	printf("exit : It is used to terminate file system \n");
	printf("truncate : It is used to remove all data from file \n");
	printf("---------------------------------------------\n");
}

void ManPage(char *Str)
{
	if(strcmp(Str,"open")== 0)
	{
		printf("Description : It is used to open an existing file \n");
		printf("Usage :      open File_Name Mode \n");
	}
	else if(strcmp(Str,"close") == 0)
	{
		printf("Description : It is used to close The existing file \n ");
		printf("Usage : close File_Name\n");
	}
	else if(strcmp(Str,"ls") == 0)
	{
		printf("Description : It is used to list out all names of the files \n ");
		printf("usage       :ls\n");
	}
	else if(strcmp(Str,"creat") == 0)
	{
		printf("Description : It is used to create the files \n ");
		printf("usage       :create file_Nane permission\n");
	}
	else if(strcmp(Str,"rm") == 0)
	{
		printf("Description : It is used to delete regular file \n");
		printf("Usage : rm File_name \n");
	}
	else if(strcmp(Str,"read") == 0)
	{
		printf("Description: Used to read data from regular file\n");
		printf("Usage:read File_name No_Of_Bytes_To_Read\n");
	}
	else if(strcmp(Str,"write") ==0)
	{
		printf("Description : It is used to write data into file \n");
		printf("Usage : write File_Description \n");
		printf("After the command please enter the data\n");
	}
	else if(strcmp(Str,"stat") == 0)
	{
		printf("Description : It is used to dispaly information of file \n");
		printf("Usage : stat File_name \n");
	}
	else if(strcmp(Str,"fstat") == 0)
	{
		printf("Description : It is used to dispaly information of file \n");
		printf("Usage : stat File_Descriptor \n");
	}
	else if(strcmp(Str,"truncate") == 0)
	{
		printf("Description : It is used to remove data from file \n");
		printf("Usage : truncate File_name \n");
	}
	else if(strcmp(Str,"closeall") == 0)
	{
		printf("Description : It is used to close all opened file \n");
		printf("Usage : closeall\n");
	}
	else if(strcmp(Str,"lseek") == 0)
	{
		printf("Description : It is used to change file offset \n");
		printf("Usage : lseek File_name ChangeInOffset StartPoint \n");
	}
	else
	{
		printf("Man page not found \n");
	}
}
int main()
{
	char Str[80];
	char Command[4][80];
	int Count = 0;
	int ret = 0;
	int fd = 0;
	char *iptr = NULL;
	printf("Customised virtul file system \n");
	SetEnvoirnment();
	
	while(1)
	{
		 printf("Marvellous VFS :> ");
		                                                      // scanf("%[^'\n']s",Str);            //fget(Str,80,stdin);
		 fgets(Str,80,stdin);
		 fflush(stdin);
		 
		 printf("Enterered command is :%s\n",Str);
		 
		 Count = sscanf(Str,"%s %s %s %s",Command[0],Command[1],Command[2],Command[3]);
		 
		 if(Count == 1)
		 {
			 if(strcmp(Command[0], "help") == 0)
			 {
				 DisplayHelp();
				 continue;
			 }
			 else if(strcmp(Command[0], "exit") == 0)
			 {
				 printf("Thank you for using Marvellous Virtual File System");
				 break;
			 }
			 else if(strcmp(Command[0],"cls") == 0)
			 {
				 system("cls");
			 }
			 else if(strcmp(Command[0],"ls") == 0)
			 {
				 LS();
			 }
			 else if(strcmp(Command[0],"closeall") == 0)
			 {
				 CloseAllFile();
				 printf("All file closed successfully \n");
				 continue;
			 }	 
			 else
			 {
				 printf("command not found !! \n");
				 continue;
			 }
		 }
		 else if(Count == 2)
		 {
			 if(strcmp(Command[0], "man") == 0)
			 { 
		         ManPage(Command[1]);
			 }
			 else if(strcmp(Command[0],"stat") == 0)
			 {
				 ret = stat_file(Command[1]);
				 
				 if(ret == -1)
					 printf("Error: Incorrect parameter\n");
				 if(ret == -2)
					 printf("Error: There is no such file\n");
				 continue;
			 }
			 else if(strcmp(Command[0],"fstat") == 0)
			 {
				 ret = fstat_file(atoi(Command[1]));
				 if(ret == -1)
					 printf("Error: Incorrect parameter\n");
				 if(ret == -2)
					 printf("Error: There is no such file\n");
				 continue;
			 }
			 else if(strcmp(Command[0],"close") == 0)
			 {
				ret = CloseFileByName(Command[1]);
				 if(ret == -1)
					 printf("Error: There is no such file\n");
				 continue;
			 }
			 else if(strcmp(Command[0],"rm") == 0)
			 {
				 rm_File(Command[1]);                                                //DeleteFile(Command[1]);    
			 }
			 else if(strcmp(Command[0],"write") == 0)
			 {
				 char arr[1024];
				 
				 printf("please enter data to write \n");
				 fgets(arr,1024,stdin);
				 
				 fflush(stdin);
				 
				 int ret = WriteFile(atoi(Command[1]),arr,strlen(arr)-1);
				 if(ret != -1)
				 {
					  printf("%d bytes gets written successfully in the file \n",ret);
				 }
			 }
			 else if(strcmp(Command[0],"truncate") == 0)
			 {
				 ret = truncate_File(Command[1]);
				 
				 if(ret == 0)
					 printf("Data from file delete successfully\n");
				 if(ret == -1)
				    printf("Error: Incorrect Parameter\n");
			 }
			 else
			 {
				 printf("Command not found !! \n");
			 }
		 }
		 else if(Count == 3)
		 {
			 if(strcmp(Command[0],"creat") == 0)
             {
				 int fd = 0;
				 fd = CreateFile(Command[1],atoi(Command[2])); //atoi is inbuild function that conver  oct to int 
				 
				 if(fd >= 0)
				 {
					 printf("File is successfully created with fd : %d",fd);
				 }
				 if(fd == -1)
				 {
					 printf("Error: Incorrect parameter \n");
				 }
				 if(fd == -2)
				 {
					 printf("Error There is no iNode");
				 }
				 if(fd == -3)
				 {
					 printf("Error: file already present");
				 }
				 if(fd == -4)
				 {
					 printf("Error: Memory allocation failure");
				 }
			 }
			 else if(strcmp(Command[0],"open") == 0)
			 {
				 fd = OpenFile(Command[1],atoi(Command[2]));
				 
				 if(fd >= 0)
					 printf("File is successfully opened with fd:%d",fd);
				 if(fd == -1)
					 printf("Error: Incorrect parameter\n");
				 if(fd == -2)
					 printf("Error: File not present \n");
				 if(fd == -3)
					 printf("Error: permission denied \n");
			 continue;
			 
		     }
			 else if(strcmp(Command[0],"read") == 0)
			 {
				 fd = GetFDFromName(Command[1]);
				 if(fd == -1)
				 {
					 printf("Error: Incorrect parameter \n");
					 continue;
				 }
				 iptr = (char *)malloc(sizeof(atoi(Command[2]))+1);
				 
				 if(iptr == NULL)
				 {
					 printf("Error: Memory allocation failure \n");
					 continue;
				 }
				ret = ReadFile(fd,iptr,atoi(Command[2]));
				 
				 if(ret == -1)
				      printf("Error: File not existing \n");
				 if(ret == -2)
					  printf("Error: Permission denied \n");
				  if(ret == -3)
				      printf("Error:Reached at end of file\n");
				 if(ret == -4)
					  printf("Error: It is not regular file \n");
		          if(ret > 0)
				  {
                      write(2,iptr,ret);
				  }
                  continue;				  
		 }
		 else
		 {
			 printf("\n Error: command not found !!!\n");
			 continue;
		 }
		 }
		 else if(Count == 4)
		 {
			 if(strcmp(Command[0],"lseek") == 0)
			 {
				 fd = GetFDFromName(Command[1]);
				 if(fd == -1)
				 {
					 printf("Error: Incorrect parameter\n");
					 continue;
				 }
				 ret = LseekFile(fd,atoi(Command[2]),atoi(Command[3]));
				 if(ret == -1)
				 {
					 printf("Error: Unable to perform lseek\n");
				 }
				 else
				 {
					 printf("Error: Command not found \n");
					 continue;
				 }
			 }
		 }
		 else
		 {
			 printf("Error: Command not found \n");
	         continue;
		 }
	}
	return 0;
}