#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

//定义hash表中的节点的类型
struct nlist{
    struct nlist *prev;
    struct nlist *next;
    char *key;
    char *value;
};

typedef struct nlist NList;

void hashtable_init(void);                        //初始化hash表对象
void hashtable_destroy(void);                     //销毁hash表对象

NList *hashtable_insert(const char *key, const char *value);   //插入一个key=value的对象
void hashtable_remove(const char *key);                 //删除一个对象，根据key
NList *hashtable_search(const char *key);               //查找一个对象，根据key
const char * hashtable_value(const char *key);          //获取一个value值，根据key

#endif //_HASHTABLE_H_

