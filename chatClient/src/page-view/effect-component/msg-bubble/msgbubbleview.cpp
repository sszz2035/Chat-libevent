#include "msgbubbleview.h"

#include <QScrollBar>

MsgBubbleView::MsgBubbleView(QWidget *parent) : ElaListView(parent){
}

void MsgBubbleView::resizeEvent(QResizeEvent *event) {
    QListView::resizeEvent(event);
    this->viewport()->update();
}
