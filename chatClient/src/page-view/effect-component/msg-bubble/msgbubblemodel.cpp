//
// Created by FU-QAQ on 2024/12/19.
//

#include "msgbubblemodel.h"

#include "msgbubbledelegate.h"

MsgBubbleModel::MsgBubbleModel(QObject *parent) {
}

MsgBubbleModel::~MsgBubbleModel() {
}

void MsgBubbleModel::addMsg(const ChatMessage &msg) {
    QStandardItem *item = new QStandardItem();
    item->setData(QVariant::fromValue(msg), MsgBubbleDelegate::MsgRole);
    appendRow(item);
}
