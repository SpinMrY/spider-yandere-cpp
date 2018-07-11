#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <thread>
#include <curl/curl.h>
#include <exception>
#include <unistd.h>

using std::string;
using std::cin;
using std::cout;
using std::endl;

struct _ImgInfo{
    string _down_url;
    string _down_title;
    int _rating;
    int _index;
};

static size_t get_curlcb(void *buffer, size_t sz, size_t nmemb, void *writer){
    string* psResponse = (string*) writer;
    size_t size = sz * nmemb;
    psResponse->append((char*) buffer, size);
    return sz * nmemb;
}
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)  {  
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);  
    return written;  
}  
   
void down_image(struct _ImgInfo info){  
    //cout<<info._down_url<<endl;
    CURL *curl_handle;
    string fn=info._down_title+".jpg";
    //cout<<info._rating<<endl;
    switch(info._rating){
    case 1:
        fn="./Safe/"+fn;
        break;
    case 2:
        fn="./Questionable/"+fn;
        break;
    case 3:
        fn="./Explicit/"+fn;
        break;
    }
    const char *pagefilename = fn.c_str();  
/*
    cout<<pagefilename<<endl;
    cout<<"IMG INFO DOWNLOAD:"<<endl;
    cout<<"TITLE "<<info._down_title<<endl;
    cout<<"URL "<<info._down_url<<endl;
    cout<<"RATING "<<info._rating<<endl;
*/      
    FILE *pagefile;  
    curl_global_init(CURL_GLOBAL_ALL);  
    curl_handle = curl_easy_init();  
    curl_easy_setopt(curl_handle, CURLOPT_URL, info._down_url.c_str());  
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);  
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);  
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);  
    pagefile = fopen(pagefilename, "wb");  
    if (pagefile) {
        cout<<"\033[1;33;44mStart Downloading\033[0m"<<endl;
        
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);  
        curl_easy_perform(curl_handle);  
        fclose(pagefile);  
    }else{
        cout<<"\033[1;33;44mOpen file error\033[0m"<<endl;
        return;
    }
    
    curl_easy_cleanup(curl_handle);  
    cout<<"\033[1;33;44mDownload successfully!\033[0m "<<info._index<<endl;
    return;
}  

string get_html_source(string url){
    string str_tmpstr;
    CURL *curl=curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_curlcb); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str_tmpstr);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    string str_res;
    if(res!=CURLE_OK){
        str_res="error";
    }
    else{
        str_res=str_tmpstr;
    }
    return str_res;
}

void kmp_get_next(string p,int next[])  {  
    int plen=p.length();  
    next[0]=-1;  
    int k=-1,j=0;  
    while(j<plen-1){  
        if(k==-1 || p[j]==p[k]){  
            k++;j++;  
            next[j]=k;  
        } 
        else{  
            k=next[k];  
        }  
    }
}

int kmp_search(string s,string p){
    int next[50005];
    memset(next,0,sizeof(next));
    kmp_get_next(p,next);
    int i=0,j=0;
    int slen=s.length(),plen=p.length();
    while(i<slen && j<plen){
        if(j==-1 || s[i]==p[j]){
            i++;j++;
        }
        else{
            j=next[j];
        }
    }
    if(j==plen)return i-j;
    else return -1;
}

_ImgInfo img_search(string _html,int index){
    struct _ImgInfo res;
    res._index=index;
    cout<<_html.length()<<endl;
    int down_url_head=kmp_search(_html,"\"highres\"")+16;
    int down_url_tail=kmp_search(_html,"Download")-2;
    if(down_url_head <=0 || down_url_tail<=0){
        cout<<"\033[1;33;44mFailed to get download url in "<<index<<"\033[0m"<<endl;
        res._down_url="error";
        return res;
    }
    string down_url=_html.substr(down_url_head,down_url_tail-down_url_head);
    cout<<"Downurl:"<<down_url_head<<' '<<down_url_tail<<endl;
    res._down_url=down_url;
    int down_title_head=kmp_search(_html,"<title>")+7;
    int down_title_tail=kmp_search(_html,"</title>")-9;
    string down_title=_html.substr(down_title_head,down_title_tail-down_title_head);
    for(int i=0;i<down_title.length();i++){
        if(down_title[i]==' '||down_title[i]=='|'||down_title[i]=='#'||down_title[i]=='/')down_title[i]='_';
    }
    cout<<down_title<<endl;
    res._down_title=down_title;
    int rat;
    if(kmp_search(_html,"Rating: Questionable")!=-1){
        rat=2;
    }
    else if(kmp_search(_html,"Rating: Safe")!=-1){
        rat=1;
    }
    else{
        rat=3;
    }
    res._rating=rat;   
    return res;
}

void get_through_index(int index){
    try{
        string _html="https://yande.re/post/show/"+std::to_string(index);
        string _html_source=get_html_source(_html);
        struct _ImgInfo _result=img_search(_html_source,index);
        cout<<"\033[1;32;43mIMG INFO:\033[0m"<<endl;
        cout<<"\033[1;32;41mTITLE "<<_result._down_title<<"\033[0m"<<endl;
        cout<<"\033[1;32;41mURL "<<_result._down_url<<"\033[0m"<<endl;
        cout<<"\033[1;32;41mRATING "<<_result._rating<<"\033[0m"<<endl;
        sleep(5);
        if(_result._down_url=="error"){
            return;
        }
        else{
            down_image(_result);
        }
    }
    catch(std::exception& e){  
        cout << "Standard exception: " << e.what() << endl;  
    }  
    return;
}

int main(){
    string now_index_html=get_html_source("https://yande.re/post");
    int now_index_pos=kmp_search(now_index_html,"/post/show/")+11;
    cout<<"\033[1;32;43mNow index:"<<now_index_html.substr(now_index_pos,6)<<"\033[0m"<<endl;
    cout<<"\033[1;32;43mEnter the start_index,end_index,delay_time and threads:\033[0m"<<endl;
    int start,end,thread,delaytime;
    cin>>start>>end>>delaytime>>thread;
    
    for(int i=start;i<=end;i=i+thread){
        for(int j=0;j<thread&&i+j<=end;j++){
            std::thread task([i,j]{
                    get_through_index(i+j);
                });
            task.detach();
        }
        sleep(delaytime);
        cout<<"\033[1;32;43m"<<thread<<" Threads OVER\033[0m"<<endl;
    }
    return 0;
}
