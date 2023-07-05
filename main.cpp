#include<dirent.h>
#include <unistd.h>
#include<stdio.h>
#include<iostream>
#include<termios.h>
//#include <sys/types.h>
#include<sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdint.h>
#include<bits/stdc++.h>
#include <fcntl.h>
#include <signal.h>



using namespace std;

stack<string> fDup;
stack<string> bDup;
string cP;
bool n_mode;


void controlFlow(string currentPath,stack<string> forward_track,stack<string> backward_track,bool cn);
void commandMode(int loc,int i,int j,int height,int vec_pos,int scr_pos,string& currentPath,vector<string> content,stack<string>& forward_track,stack<string>& backward_track);
void cursorNavigation(int i,int j,int height,int vec_pos,int scr_pos,vector<string> content,string currentPath,stack<string> forward_track,stack<string> backward_track);
vector<string> getContentLists(DIR *dir);
void showContent(int i,int j,string currentPath,vector<string> content,int h,bool pr);
bool delete_directory(string srcDir,string currentPath);

void set_pos(int x,int y)
{
    printf("\033[%d;%dH",x,y);
} 

void setGlobal(stack<string> forwardStack,stack<string> backwardStack,string currentPath,bool md)
{
    //cout<<"setting global variables"<<endl;
    fDup = forwardStack;
    bDup = backwardStack;
    cP = currentPath;
    n_mode = md;
    // 1 means command mode
}

void do_resize(int dummy)
{
    //cout<<"Resize Detcted"<<endl;
    // set_pos(16,0);
    // cout<<fDup.size()<<" "<<bDup.size()<<" "<<cP<<" "<<n_mode<<endl;
    // set_pos(1,0);
    controlFlow(cP,fDup,bDup,n_mode);
}

int isDir(string path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    if (S_ISDIR(st.st_mode))
        return 1;
    else
        return 0;
}



vector<string> split(string s, char delim)
{
   vector<string> res;
    string temp = "";

    for(int i = 0 ; i < s.size() ; i++)
    {
        if(s[i] == delim)
            {
                if(temp.size() != 0)
                    res.push_back(temp);
                temp="";
            }
        else
            temp += s[i];
    }

    res.push_back(temp);
    return res;
}

string getHome()
{
    string curr_dir = get_current_dir_name();
    //vector<string> path = split(curr_dir,"/");
    vector<string> path = split(curr_dir,'/');
    return ( '/' + path[0] + '/' + path[1] + '/');
}



void openFile(const char *filename)   //converting string into const char* is allowed
{
                    

    if (fork() == 0) {
		execl("/usr/bin/xdg-open", "xdg-open", filename, (char *)0);
        exit(0);
		
	}
}

int getch(void)
{
 int ch;
 struct termios oldtio;
 struct termios newtio;
 tcgetattr(0, &oldtio); //0 is file discriptor for standard input
 
 newtio = oldtio; 
 newtio.c_lflag &= ~(ICANON | ECHO);   //switch to noncanonical and nonecho mode 
 tcsetattr(0, TCSANOW, &newtio);

 ch=cin.get();                         //Take the input
                   
 if(tcsetattr(0, TCSANOW, &oldtio))    //switch to cananical mode
    {
        fprintf(stderr,"Set attributes failed");
        exit(1);
    }
 
    return ch; 
}

mode_t get_mode(string srcPath)
{
    struct stat sn;
    stat(srcPath.c_str(), &sn);
    mode_t m=0;
    
    
    
    m = m | ((sn.st_mode & S_IROTH)?0004:0);
    m = m | ((sn.st_mode & S_IWOTH)?0002:0);
    m = m | ((sn.st_mode & S_IXOTH)?0001:0);

    m = m | ((sn.st_mode & S_IRUSR)?0400:0);
    m = m | ((sn.st_mode & S_IWUSR)?0200:0);
    m = m | ((sn.st_mode & S_IXUSR)?0100:0);

    m = m | ((sn.st_mode & S_IRGRP)?0040:0);
    m = m | ((sn.st_mode & S_IWGRP)?0020:0);
    m = m | ((sn.st_mode & S_IXGRP)?0010:0);
    

    return m;
}

string getFullPath(string path,string currentPath)
{
    string cur_dir = currentPath;
    string abs_path = "";
    if (path[0] == '~') 
        abs_path = getHome() + path.substr(1, path.length());
   
    else if (path[0] == '/') 
        abs_path =  path;
    
    else if (path[0] == '.' && path[1] == '/') 
        abs_path = cur_dir + path.substr(1, path.length());
    
    else 
        abs_path = cur_dir + "/" + path;
    
    //return abs_path;
    //set_pos(25,0);
    //cout<<realpath(abs_path.c_str(),NULL);
    char *st = realpath(abs_path.c_str(),NULL);
    if(st != NULL){return string(st);}
    else{return "";}
}

bool copyFile(string srcPath,string destPath,bool flag)
{
    // cout<<srcPath<<endl;
    // cout<<destPath<<endl;

    char buf;
	int fd_one, fd_two;

	fd_one = open(srcPath.c_str(), O_RDONLY);

	if (fd_one == -1)
	{
            cout<<endl;
            printf("Error in performing operation");
		    close(fd_one);

		return false;
	}
    mode_t m = get_mode(srcPath);

	fd_two = open(destPath.c_str(), 
				  O_WRONLY | O_CREAT,
				  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,m);
	
	while(read(fd_one, &buf, 1))
	{
		write(fd_two, &buf, 1);
	}


	close(fd_one);
	close(fd_two);

    return true;
}

bool copy(vector<string> command, string currentPath)
{
    bool flag = true;
    string pwd = currentPath;

    string destFolder = getFullPath(command[command.size() - 1],currentPath);
    //cout<<endl<<destFolder<<endl;
    if(destFolder.size() == 0){return false;}
    
    for(int i = 1 ; i < command.size() - 1; i++)
    {
        //abs path of source
        string sourcePath = getFullPath(command[i],currentPath);
        //cout<<sourcePath<<endl;
        //vector<string> names=split(command[i],"/");
        vector<string> names=split(command[i],'/');

        string destPath = destFolder +"/"+ names[names.size()-1];
        //cout<<destPath;
        if(!copyFile(sourcePath,destPath,true)){flag = false; return flag;}
       
    }

   return flag;
    
}

bool copy_d(string srcDir, string destDir, string currentPath)
{
   bool flag = true;
   DIR *dir;
   dir = opendir(srcDir.c_str());
   //vector<string> names=split(srcDir,"/");
   vector<string> names=split(srcDir,'/');
   string destPath= destDir +"/"+ names[names.size()-1];
   
   //cout<<"2"<<destPath<<endl;

   mode_t m= get_mode(srcDir);
   if(mkdir(destPath.c_str(),m) == -1) 
   {
        cout<<endl<<"Could not create DIRECTORY"<<endl;
        flag=false;
        return false;
   }

   vector<string> content = getContentLists(dir);

   for(int i = 2 ; i < content.size() ; i++)
   {
        string newPath = srcDir + "/" + content[i];
        if(isDir(newPath))
        {
            if(!copy_d(newPath,destPath,newPath)){return false;}
        }
        else{
            vector<string> cmd;
            //cout<<"2"<<newPath<<" "<<destPath<<endl;
            cmd.push_back("copy");
            cmd.push_back(newPath);
            cmd.push_back(destPath);
            flag = copy(cmd,currentPath);
        }
   }

   return flag;

}

bool move(vector<string> command, string currentPath )
{
    //copy(command,currentPath);
    bool flag1 = true;
    bool flag2 = true;

     string srcPath = getFullPath(command[1],currentPath);
     if(isDir(srcPath))
         {
            //cout<<"1"<<srcPath<<" "<<getFullPath(command[command.size() - 1],currentPath)<<endl;
            string destDir = getFullPath(command[command.size() - 1],currentPath);
            mode_t m= get_mode(destDir);

            // if(mkdir(destDir.c_str(),m) == -1) 
            // {
            //         cout<<"Could not create DIRECTORY at start"<<endl;
            // }

            flag1 = copy_d(srcPath,destDir,currentPath);
            if(!flag1){return false;}

            //cout<<"Before Deleting path"<<srcPath<<endl;
            flag2 = delete_directory(srcPath,currentPath);
            if(!flag2){return false;}

            
            rmdir(srcPath.c_str());

            return true;
         }


         else{
            //cout<<endl<<"Reached in copy"<<endl;
            flag1 = copy(command,currentPath);
            if(!flag1){return false;}
            string pwd = currentPath;

            //deleting now
            for(int i = 1 ; i < command.size() - 1 ;i++)
            {
                string sourcePath = getFullPath(command[i],currentPath);
                int status = remove(sourcePath.c_str());
                if(status){return false;}
            }
            
            return true;

         }
   
    
}

void rename(vector<string> command, string currentPath)
{
    string pwd = currentPath;
    string oldFileName = pwd + '/' + command[1];
    string newFileName = pwd + '/' + command[2];
    int status = rename(oldFileName.c_str() , newFileName.c_str());
    if(!status){cout<<endl; cout<<"File renamed succesfully"<<endl;}
    else{cout<<endl; cout<<"Error in renaming file"<<endl;}

}

bool search_command(string tobeSearched,string current_dir)
{
    
    DIR *dir;
    dir = opendir(current_dir.c_str());
    vector<string> content = getContentLists(dir);
    struct stat statbuf;

    
    if(content.size() < 2){return false;}
    for(int i = 2 ; i < content.size() ; i++)
    {

        //if(content[i].at(0) == '.') {continue;}
        if(content[i] == tobeSearched){return true;}
        string src = current_dir + '/' + content[i];
        stat(src.c_str(),&statbuf); 
        if (S_ISDIR(statbuf.st_mode))
        {
            //string nextDir ="";
            //nextDir += current_dir + '/' + content[i];
            //cout<<nextDir<<endl;
            if (search_command(tobeSearched,src)) return true;
        }


    }

    return false;
    // for(int i = 0 ; i < content.size() ; i++)
    // {
    //     cout<<content[i]<<endl;
    // }
}

void createFile(vector<string> command,vector<string> content, string current_dir)
{
    string destPath = "";
    if(command.size() == 2){destPath =  current_dir + "/" + command[1];}
    else{destPath = getFullPath(command[2],current_dir) + "/" + command[1];}


    if(creat(destPath.c_str(),0600)==-1)
            {cout<<endl;
            printf("couldn't create file"); }
    else{cout<<endl; cout<<"Created"<<endl;}
   
        

    
}

void createDirectory(vector<string> command,vector<string> content, string current_dir)
{
    string destPath = "";
    if(command.size() == 2){destPath =  current_dir + "/" + command[1];}
    else{destPath = getFullPath(command[2],current_dir) + "/" + command[1];}

    // cout<<destPath;

    if(mkdir(destPath.c_str(),0775)==-1)
    {       cout<<endl;
            printf("couldn't create directory");
            cout<<endl;}
    

    else{cout<<endl; cout<<"Created"<<endl;}
}

bool delete_file(string src,string currentPath)
{
    //cout<<endl<<src<<endl;
    string sourcePath = getFullPath(src,currentPath);
    /*cout<<endl<<sourcePath<<endl;
    int fd_one = open(sourcePath.c_str(), O_RDONLY);
    if (fd_one == -1)
	{
            cout<<endl;
            printf("Error in performing operation");
		    close(fd_one);

		return false;
	}*/
    int status = remove(sourcePath.c_str());
    if(!status){return true;}
    return false;
    
   
}

bool delete_directory(string srcDir,string currentPath)
{
   bool flag1 = true;
   bool flag2 = true;
   DIR *dir;
   srcDir = getFullPath(srcDir,currentPath);
   //cout<<"1"<<srcDir<<endl;
   dir = opendir(srcDir.c_str());
   if(dir == NULL){return false;}

   vector<string> content = getContentLists(dir);
   
   for(int i = 2 ; i < content.size() ; i++)
   {
        if(string(content[i]) == "." || string(content[i]) == ".."){continue;}
        string pth = srcDir + "/" + content[i];
        //string pth = getFullPath(content[i],currentPath);
        //cout<<endl<<pth<<endl;

        if(isDir(pth))
        {
            
            flag1 = delete_directory(pth , currentPath+"/"+content[i]);
            if(!flag1){return false;}
    
            rmdir(pth.c_str());
            //cout<<"Directory "<<pth<<" removed "<<endl;
        }
        else{
            flag2 = delete_file(pth,currentPath);
            if(!flag2){return false;}
        }
   }

   return true;
}

void goto_command(vector<string> command,string &currentPath,stack<string> &forward_track,stack<string> &backward_track)
{
    string path = getFullPath(command[1],currentPath);
    // set_pos(25,0);
    // cout<<endl<<path<<endl;
    backward_track.push(currentPath);
    while(forward_track.size() != 0)
        forward_track.pop();
    
    currentPath = path;

    // cout<<endl;
    // cout<<path<<endl;
    
    

}
void parseCommand(string s,string &currentPath,vector<string> content,stack<string> &forward_track,stack<string> &backward_track,int i,int j,int height,int vec_pos,int scr_pos)
{
    
    //vector<string> parse = split(s," ");
    vector<string> parse = split(s,' ');

    if(parse.size() < 3 && parse[0] != "search" && parse[0] != "create_file" && parse[0].at(0) != 'd' && parse[0] != "create_dir" && parse[0] != "goto" && parse[0] != "quit")
    { 
         set_pos(height+6,1);
         printf("*********   Invalid COMMAND.. Press Enter to continue   **************");
         char c;
         c=getch();
         if(c == 10){
         set_pos(height+5,18);
         printf("\e[0J");}
         controlFlow(currentPath,forward_track,backward_track,false);
         commandMode(20,i,j,height,vec_pos,scr_pos,currentPath,content,forward_track,backward_track);
    }

    if(parse[0] == "goto")
    {
        goto_command(parse,currentPath,forward_track,backward_track);
        controlFlow(currentPath,forward_track,backward_track,false);
        cout<<endl;
        set_pos(height+5,0);
        cout<<"Press ENTER to continue..........";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,0);
        printf("\e[0J");}
         struct winsize win;
             ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
             int rowsize = win.ws_row - 1;
             int colsize = win.ws_col;
             set_pos(rowsize-2,0);
             cout<<"Current Directory : ";
             cout<<currentPath;
             set_pos(height+5,0);
        return;

        //controlFlow(currentPath,forward_track,backward_track,false);
    }

    if(parse[0] == "copy")
    {
        //for(int i = 0 ; i < parse.size() ; i++){cout<<endl;cout<<parse[i]<<endl;}

         string srcPath = getFullPath(parse[1],currentPath);
         if(isDir(srcPath))
         {
            //cout<<"1"<<srcPath<<" "<<getFullPath(parse[parse.size() - 1],currentPath)<<endl;
            string destDir = getFullPath(parse[parse.size() - 1],currentPath);
            mode_t m= get_mode(destDir);

            // if(mkdir(destDir.c_str(),m) == -1) 
            // {
            //         cout<<"Could not create DIRECTORY at start"<<endl;
            // }

            if(copy_d(srcPath,destDir,currentPath)){cout<<endl; cout<<"Success"<<endl;}
            else{cout<<endl; cout<<"Failure"<<endl;}
         }

         else{
         
         if(copy(parse,currentPath)){cout<<endl; cout<<"Success"<<endl;}
         else{cout<<endl; cout<<"Failure"<<endl;}
         }
         cout<<"Press ENTER to continue..........";
         char c;
         c=getch();
         if(c == 10){
         set_pos(height+5,18);
         printf("\e[0J");}

         controlFlow(currentPath,forward_track,backward_track,false);
    }

    else if(parse[0] == "move")
    {
        if(move(parse,currentPath)){cout<<endl; cout<<"Success"<<endl;}
        else{cout<<endl; cout<<"Failure"<<endl;}
        cout<<"Press ENTER to continue..........";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}

        controlFlow(currentPath,forward_track,backward_track,false);
    }

    else if(parse[0] == "rename")
    {
        rename(parse,currentPath);
        

        cout<<"Press ENTER to continue..........";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}

        controlFlow(currentPath,forward_track,backward_track,false);
    }
    

    else if(parse[0] == "search")
    {
        string curr_dir = currentPath;
        bool res = search_command(parse[1],curr_dir);
        cout<<endl;
        if(res) cout<<"TRUE"<<endl;
        else cout<<"FALSE"<<endl;
        
        cout<<"Press ENTER to continue.............";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}

        controlFlow(currentPath,forward_track,backward_track,false);

    }

    else if(parse[0] == "create_file")
    {
        createFile(parse,content,currentPath);

        cout<<"Press ENTER to continue..........";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}

        controlFlow(currentPath,forward_track,backward_track,false);
        
    }

    else if(parse[0] == "create_dir")
    {
        createDirectory(parse,content,currentPath);
        
        cout<<"Press ENTER to continue..........";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}

        controlFlow(currentPath,forward_track,backward_track,false);
    }

    else if(parse[0] == "delete_dir")
    {
       if(delete_directory(parse[1],currentPath)){cout<<endl; cout<<"Success"<<endl;}
       else{cout<<endl; cout<<"Failure"<<endl;}

       string dir_path = getFullPath(parse[1],currentPath);
       //cout<<"****"<<dir_path<<"***********"<<endl;
       rmdir(dir_path.c_str());

        cout<<"Press ENTER to continue..........";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}

        controlFlow(currentPath,forward_track,backward_track,false);
    }

    else if(parse[0] == "delete_file")
    {
        
        if(delete_file(parse[1],currentPath)){cout<<endl; cout<<"Success"<<endl;}
        else{cout<<endl; cout<<"Failure"<<endl;}
        cout<<"Press ENTER to continue..........";
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}

        controlFlow(currentPath,forward_track,backward_track,false);
    }

    else if(parse[0] == "quit")
    {
        exit(0);
    }

    else{
        set_pos(height+6,1);
        printf("**************   INVALID COMMAND Press Enter to continue   **************");
        char c;
        c=getch();
        if(c == 10){
        set_pos(height+5,18);
        printf("\e[0J");}
    }
    
}


void commandMode(int loc,int i,int j,int height,int vec_pos,int scr_pos,string& currentPath,vector<string> content,stack<string>& forward_track,stack<string>& backward_track)
{
    setGlobal(forward_track,backward_track,currentPath,false);
    set_pos(height+5,1);
    printf("\33[2K\r");
    cout<<"COMMAND MODE : $ ";
    int y = 18;
    set_pos(height+5,y);

    string s="";
    char c;
    
    while(1)
    {
        char c = getch();
        if(c != 127 && c != 10)
        {
            s+=c;
            set_pos(height+5,y);
            cout<<s;
        }
        if( c == 27)
        {
            controlFlow(currentPath,forward_track,backward_track,true);
            // cursorNavigation(i,j,height,vec_pos,scr_pos,content,currentPath,forward_track,backward_track);
        }
        if(c == 127)
        {
            s=s.substr(0,s.length()-1);

            set_pos(height+5,y);
            printf("\e[0J");

             struct winsize win;
             ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
             int rowsize = win.ws_row - 1;
             int colsize = win.ws_col;
             set_pos(rowsize-2,0);
             cout<<"Current Directory : ";
             cout<<currentPath;
             set_pos(height+5,y);
             cout<<s;



        }

        if(c == 10 && s.size() == 0)
        {
            commandMode(20,i,j,height,vec_pos,scr_pos,currentPath,content,forward_track,backward_track);
        }

        else if(c == 10)
        {
            break;
        }
       
    }
    
      parseCommand(s,currentPath,content,forward_track,backward_track,i,j,height,vec_pos,scr_pos);
      setGlobal(forward_track,backward_track,currentPath,false);

    
      commandMode(20,i,j,height,vec_pos,scr_pos,currentPath,content,forward_track,backward_track);
    }

    
void showContent(int start,int end,string currentPath,vector<string> content,int h,bool pr)
{
   
    set_pos(1,0);
    printf("\33c");

    if(pr) {
    set_pos(h+5,1);
    cout<<"*********************Normal Mode****************************";}
    
    struct winsize win;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
    int rowsize = win.ws_row - 1;
    int colsize = win.ws_col;
    set_pos(rowsize-2,0);
    cout<<"Current Directory : ";
    
    string newPath = string(realpath(currentPath.c_str(),NULL));
    cout<<newPath;
    //cout<<rowsize<<" "<<colsize;


    set_pos(1,0);





    //set_pos(1,0);

    
    
    struct stat     statbuf;
    struct passwd  *pwd;
    struct group   *grp;
    struct tm      *tm;
    char datestring[256];
    
    int size = content.size();

    for(int i = start ; i < end ; i++)
    {
    stat((currentPath+"/"+content[i]).c_str(),&statbuf); 
    
     
      string perm="";

          if (S_ISDIR(statbuf.st_mode))
          {
            perm+="d";
          }
          else{
            perm+="-";
          }
          
          perm+=((statbuf.st_mode & S_IRUSR) ? "r" : "-");
	      perm+=((statbuf.st_mode & S_IWUSR) ? "w" : "-");
	      perm+=((statbuf.st_mode & S_IXUSR) ? "x" : "-");
	      perm+=((statbuf.st_mode & S_IRGRP) ? "r" : "-");
	      perm+=((statbuf.st_mode & S_IWGRP) ? "w" : "-");
	      perm+=((statbuf.st_mode & S_IXGRP) ? "x" : "-");
          perm+=((statbuf.st_mode & S_IROTH) ? "r" : "-");
	      perm+=((statbuf.st_mode & S_IWOTH) ? "w" : "-");
	      perm+=((statbuf.st_mode & S_IXOTH) ? "x" : "-");

    cout<<perm<<" ";

    /* Print out owner's name if it is found using getpwuid(). */
    if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
        printf("%-8.8s", pwd->pw_name);
   
    /* Print out group name if it is found using getgrgid(). */
    if ((grp = getgrgid(statbuf.st_gid)) != NULL)
        printf("%-8.8s", grp->gr_name);
   
    /* Print size of file. */
    printf("%9jd  ", (intmax_t)statbuf.st_size);


    tm = localtime(&statbuf.st_mtime);


     /* Get localized date string. */
    strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);

    //printf("%s %s\n", datestring, content[i].c_str());
    printf("%s", datestring);
    cout<<"  ";
    string s = content[i];
    if(s.size() > 10)
    {
        s=s.substr(0,12);
        s=s+"...";
    }
    cout<<s<<endl;


    
}

}


void cursorNavigation(int i,int j,int height,int vec_pos,int scr_pos,vector<string> content,string currentPath,stack<string> forward_track,stack<string> backward_track)
{
    setGlobal(forward_track,backward_track,currentPath,true);

    set_pos(height+5,1);
    cout<<"*********************Normal Mode****************************";
    int row = 1;
    set_pos(scr_pos,0);
    

    while(1)
    {
        
	    int c = getch();

        switch( c ) {

        case 27:
               getch();
               switch(getch()){
                 case 65:// Upper Key

                    if(vec_pos == 0){break;}
                    if(vec_pos == i)
                    {
                        i=i-1;
                        j=j-1;
                        showContent(i,j,currentPath,content,height,true);
                        set_pos(1,0);
                    }
                    else{
                        scr_pos--;
                        set_pos(scr_pos,0);
                    }
                    vec_pos--;
                        setGlobal(forward_track,backward_track,currentPath,true);


                    break;

                 case 66: //Lower Key
                    if(vec_pos == content.size() -1){break;}
                    vec_pos++;
                    if(scr_pos == height)
                    {
                        i=i+1;
                        j=j+1;
                        showContent(i,j,currentPath,content,height,true);
                        set_pos(scr_pos,0);
                    }
                    else{
                        scr_pos++;
                        set_pos(scr_pos,0);
                    }
                        setGlobal(forward_track,backward_track,currentPath,true);

                      break;

                 case 67: //Forward Key
                    if(!forward_track.empty())
                    {
                        
                        string newPath = forward_track.top();
                        forward_track.pop();
                        backward_track.push(currentPath);
                        setGlobal(forward_track,backward_track,currentPath,true);
                        controlFlow(newPath,forward_track,backward_track,true);
                    }
                        

                    break;
                 case 68: //Backward Key
                    if(!backward_track.empty())
                    {
                        forward_track.push(currentPath);
                        string newPath = backward_track.top();
                        backward_track.pop();
                        setGlobal(forward_track,backward_track,currentPath,true);
                        controlFlow(newPath,forward_track,backward_track,true);
                    }
                    break;

                

               }
               break;

        case 127: //Backspace Key
             backward_track.push(currentPath);
             while(forward_track.size() != 0){forward_track.pop();}
             setGlobal(forward_track,backward_track,currentPath,true);
             controlFlow(currentPath+"/..",forward_track,backward_track,true);
             break;

        case 104: //h key
            backward_track.push(currentPath);
            while(forward_track.size() != 0){forward_track.pop();}
            setGlobal(forward_track,backward_track,currentPath,true);
            controlFlow(getHome(),forward_track,backward_track,true);
            break;

        case 58: //Command Mode

            commandMode(20,i,j,height,vec_pos,scr_pos,currentPath,content,forward_track,backward_track);
            break;

         case 113: //Exiting code
                exit(0);
                break;

        case 10: //Enter Key
            
            struct stat sb;
            string newPath="";
            newPath += currentPath + '/' + content[vec_pos];
            newPath = realpath(newPath.c_str(),NULL);

            stat(newPath.c_str(),&sb);
            string df = "";
            
            if (S_ISDIR(sb.st_mode))
            {
                df += "d";
                
            }
            else{
                df += "-";
            }
 
            if(df == "d"){
                
                backward_track.push(currentPath);
                while(forward_track.size() != 0){forward_track.pop();}
                setGlobal(forward_track,backward_track,currentPath,true);
                controlFlow(newPath,forward_track,backward_track,true);
                }

                 else{
                    openFile(newPath.c_str());
				    }
			
            break;

            
            

            

           

        }

    }
    
}


vector<string> getContentLists(DIR *dir)
{
    vector<string> content;
    struct dirent *ent;
     while ((ent = readdir (dir)) != NULL) {
         content.push_back(ent->d_name);
    }
    
    

    sort(content.begin(),content.end());
    return content;
}


void  controlFlow(string currentPath,stack<string> forward_track,stack<string> backward_track,bool cn)
{

   DIR *dir;
   dir = opendir(currentPath.c_str());
   //cout<<currentPath<<" Dir "<<dir<<endl;
  
  
   vector<string> content = getContentLists(dir);

   //cout<<content.size()<<endl;
   struct winsize win;
   ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
   int rowsize = win.ws_row - 1;
   int colsize = win.ws_col;
   int h = rowsize/3;
   if(content.size() < h){h = content.size();}  
   int i = 0;
   int j = h;
   int vec_pos=0;
   int scr_pos = 1;
 
   bool pr = cn;

   
   
   closedir(dir);

   showContent(i,j,currentPath,content,h,pr);

   if(cn)
        cursorNavigation(i,j,h,vec_pos,scr_pos,content,currentPath,forward_track,backward_track);

}

int main()
{
    // Program Starting from Normal Mode
   signal(SIGWINCH, do_resize);
   stack<string> forward_track;
   stack<string> backward_track;
   string s = get_current_dir_name();
   //string s = "/home/hsahuja/Documents/AOS_Projectjb/../Resume";
   controlFlow(s,forward_track,backward_track,true);
   return 0;
}

  
