/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "item_point.h"
#include "../common_features/logger.h"

#include "../common_features/mainwinconnect.h"


ItemPoint::ItemPoint(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    gridSize=32;
    gridOffsetX=0;
    gridOffsetY=0;
    isLocked=false;

    animatorID=-1;
    scene=NULL;
    imageSize = QRectF(0,0,32,32);
    this->setData(9, QString::number(32));
    this->setData(10, QString::number(32));
}


ItemPoint::~ItemPoint()
{
    //WriteToLog(QtDebugMsg, "!<-BGO destroyed->!");
    //if(timer) delete timer;
}

void ItemPoint::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if(scene->DrawMode)
    {
        unsetCursor();
        ungrabMouse();
        this->setSelected(false);
        return;
    }
    QGraphicsItem::mousePressEvent(mouseEvent);
}

void ItemPoint::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
    QGraphicsItem::contextMenuEvent(event);
}

QRectF ItemPoint::boundingRect() const
{
    return imageSize;
}

void ItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if(scene==NULL)
    {
        painter->setPen(QPen(QBrush(Qt::yellow), 2, Qt::SolidLine));
        painter->setBrush(Qt::yellow);
        painter->setOpacity(0.5);
        painter->drawRect(1,1,imageSize.width()-2,imageSize.height()-2);
        painter->setOpacity(1);
        painter->setBrush(Qt::transparent);
        painter->drawRect(1,1,imageSize.width()-2,imageSize.height()-2);
    }
    else
    {
       painter->drawPixmap(scene->pointAnimation->image().rect(), scene->pointAnimation->image(), scene->pointAnimation->image().rect());
    }

    if(this->isSelected())
    {
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine));
        painter->drawRect(1,1,imageSize.width()-2,imageSize.height()-2);
        painter->setPen(QPen(QBrush(Qt::white), 2, Qt::DotLine));
        painter->drawRect(1,1,imageSize.width()-2,imageSize.height()-2);
    }
}

void ItemPoint::setScenePoint(WldScene *theScene)
{
    scene = theScene;
}

