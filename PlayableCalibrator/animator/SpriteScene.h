/*
 * SMBX64 Playble Character Sprite Calibrator, a free tool for playble srite design
 * Copyright (c) 2014 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef SPRITESCENE_H
#define SPRITESCENE_H
#include <QGraphicsScene>
#include <QtWidgets>


class SpriteScene : public QGraphicsScene
{
    Q_OBJECT
public:
    int mCurrentFrameX;
    int mCurrentFrameY;

    SpriteScene(QObject *parent = 0);
    void draw( );
    QPoint pos() const;
    void setFrame(int, int y);
    void setSquare(int x, int y, int h, int w);
    QPixmap mSpriteImage;

private:
    QGraphicsPixmapItem * croc;
    QGraphicsRectItem * sizer;
    QPoint mPos;
    int mXDir;
    int x, y, dir;
    QPixmap currentImage;
};

#endif // SPRITESCENE_H
