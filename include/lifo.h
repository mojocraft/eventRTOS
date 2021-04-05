/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __INCLUDE_LIFO_H__
#define __INCLUDE_LIFO_H__

#include "slist.h"

/*********************************************************
 *@类型说明：
 *
 *[lifo_t]：使用单循环链表实现的后进先出队列
 *********************************************************/
typedef struct lifo_s
{
    /* 用于实现后进先出队列的链表 */
    slist_t list;
} lifo_t;


/************************************************************
 *@简介：
 ***后进先出队列结构体静态初始化
 *
 *@用法：
 ***lifo_t lifo = LIFO_STATIC_INIT(lifo);
 *
 *@参数：
 *[head]：后进先出队列变量名，非地址
 *************************************************************/
#define LIFO_STATIC_INIT(lifo)  {SLIST_STATIC_INIT((lifo).list)}


/************************************************************
 *@简介：
 ***获取lifo中的单循环链表，用于遍历lifo
 *
 *@参数：
 *[lifo]：后进先出队列
 *
 *@返回：后进先出队列的单循环链表
 *************************************************************/
#define LIFO_LIST(lifo)      (&(lifo)->list)


/************************************************************
 *@简介：
 ***获取lifo中的链表，用于遍历lifo
 *
 *@参数：
 *[lifo]：后进先出队列
 *
 *@返回：后进先出队列的链表
 *************************************************************/
#define LIFO_OF_LIST(slist)      (container_of(slist, lifo_t, list))


/*********************************************************
 *@简要：
 ***获取后进先出队列的顶部节点
 *
 *@约定：
 ***1、lifo不是空指针
 ***2、lifo非空
 *
 *@参数：
 *[lifo]：后进先出队列
 *
 *@返回类型：
 *[slist_node_t *]：单循环链表的节点
 *
 *@返回：后进先出队列的顶部节点
 **********************************************************/
#define LIFO_TOP(lifo) SLIST_NODE_NEXT(SLIST_HEAD(&(lifo)->list))


/*********************************************************
 *@简要：
 ***后进先出队列初始化
 *
 *@约定：
 ***1、lifo不是空指针
 ***2、不可对正在使用的lifo进行初始化
 *
 *@参数：
 *[lifo]：后进先出队列
 **********************************************************/
static force_inline void lifo_init(lifo_t *lifo)
{
    slist_init(&lifo->list);
}


/*********************************************************
 *@简要：
 ***判断后进先出队列是否为空
 *
 *@约定：
 ***1、lifo不是空指针
 *
 *@参数：
 *[lifo]：后进先出队列
 *
 *@返回值：
 *[true]：后进先出队列为空
 *[false]：后进先出队列非空
 **********************************************************/
#define lifo_is_empty(lifo) slist_is_empty(&(lifo)->list)


/*********************************************************
 *@简要：
 ***后进先出队列入队
 *
 *@约定：
 ***1、lifo与node不是空指针
 ***2、node为已删除的节点
 *
 *@参数：
 *[lifo]：后进先出队列
 *[node]：需要入队的节点
 **********************************************************/
static force_inline void lifo_push(lifo_t *lifo, slist_node_t *node)
{
    slist_node_insert_next(SLIST_HEAD(&lifo->list), node);
}


/*********************************************************
 *@简要：
 ***后进先出队列出队
 *
 *@约定：
 ***1、lifo不是空指针
 ***2、lifo非空
 *
 *@参数：
 *[lifo]：后进先出队列
 *
 *@返回：出队的节点
 **********************************************************/
static force_inline slist_node_t* lifo_pop(lifo_t *lifo)
{
    return slist_node_del_next(SLIST_HEAD(&lifo->list));
}


/*********************************************************
 *@简要：
 ***在后进先出队列的节点处插入下一个节点
 *
 *@约定：
 ***1、node和next_node不是空指针
 ***2、node为后进先出队列中的节点
 ***3、next_node为已删除的节点
 *
 *@参数：
 *[node]：后进先出队列中的节点
 *[next_node]：需要插入的节点
 **********************************************************/
static force_inline void lifo_node_insert_next(slist_node_t *node, slist_node_t *next_node)
{
    slist_node_insert_next(node, next_node);
}


/*********************************************************
 *@简要：
 ***移除后进先出队列中节点的下一个节点，
 ***该函数通常在遍历过程中使用
 *
 *@约定：
 ***1、node不是空指针
 ***2、node处于后进先出队列中
 ***3、node不是后进先出队列的尾节点
 *
 *@参数：
 *[node]：后进先出队列中的节点
 *
 *@返回：被移除的节点
 **********************************************************/
static force_inline slist_node_t* lifo_node_del_next(slist_node_t *node)
{
    return slist_node_del_next(node);
}


/*********************************************************
 *@简要：
 ***从后进先出队列中移除节点
 *
 *@约定：
 ***1. lifo与node不是空指针
 *
 *@参数：
 *[lifo]：后进先出队列
 *
 *@返回值：
 *[true]：成功从队列中移除这个节点
 *[false]：这个节点不在这个队列之中
 **********************************************************/
static force_inline bool lifo_del_node(lifo_t *lifo, slist_node_t *node)
{
    return slist_del_node(&lifo->list, node);
}


/*********************************************************
 *@简要：
 ***从后进先出队列中安全插入节点
 *
 *@约定：
 ***1、node为后进先出队列中的节点
 ***2、next_node为已删除的节点
 *
 *@参数：
 *[node]: 操作的节点
 *[next_node]: 要插入的下一个节点
 *[safe_node]: 遍历过程中使用的安全节点
 **********************************************************/
static force_inline void lifo_node_insert_next_safe(slist_node_t *node, slist_node_t *next_node, slist_node_t **safe_node)
{
    slist_node_insert_next_safe(node, next_node, safe_node);
}


/*********************************************************
 *@简要：
 ***从后进先出队列中安全移除下一个节点
 *
 *@约定：
 ***1、node为后进先出队列中的节点
 ***2、node不是尾节点
 *
 *@参数：
 *[node]: 操作的节点
 *[safe_node]: 遍历过程中使用的安全节点
 *
 *@返回值: 被移除的下一个节点
 **********************************************************/
static force_inline slist_node_t *lifo_node_del_next_safe(slist_node_t *node, slist_node_t **safe_node)
{
    return slist_node_del_next_safe(node, safe_node);
}


/*********************************************************
 *@简要：
 ***从后进先出队列中安全移除节点
 *
 *@参数：
 *[lifo]：后进先出队列
 *[del_node]: 被移除的节点
 *[safe_node]: 遍历过程中使用的安全节点
 *
 *@返回值:
 *[true]: 成功从后进先出队列中移除节点
 *[false]: 当前节点不在后进先出队列之中
 **********************************************************/
static force_inline bool lifo_del_node_safe(lifo_t *lifo, slist_node_t *del_node, slist_node_t **safe_node)
{
    return slist_del_node_safe(LIFO_LIST(lifo), del_node, safe_node);
}


/*********************************************************
 *@简要：
 ***将后进先出队列所有节点转移至接收队列的头部，
 ***转移完成后，原后进先出队列变成空
 *
 *@参数：
 *[lifo]：被转移的后进先出队列
 *[recv_lifo]: 接收节点的后进先出队列
 **********************************************************/
static force_inline void lifo_nodes_transfer_to(lifo_t *lifo, lifo_t *recv_lifo)
{
    slist_nodes_transfer_to(LIFO_LIST(lifo), LIFO_LIST(recv_lifo));
}

#endif /* __INCLUDE_LIFO_H__ */
