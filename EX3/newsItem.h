//
// Created by arie1 on 5/21/2023.
//

#ifndef NEWSITEM_H
#define NEWSITEM_H

// create enum for the type of news
typedef enum {
    sports,
    weather,
    news
} newsType;

// create a struct to hold a newsItem
typedef struct {
    int id;
    newsType type;
    int numOfType;
} newsItem;

#endif //NEWSITEM_H