/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2016 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "../lvl_npc.h"
#include "../lvl_player.h"
#include "../lvl_block.h"
#include "../lvl_physenv.h"
#include "../lvl_bgo.h"
#include <data_configs/config_manager.h>
#include <common_features/number_limiter.h>
#include <common_features/maths.h>

#include "../collision_checks.h"

#define DISABLE_OLD_SPEEDADD

#ifdef OLD_COLLIDERS
void LVL_Npc::LEGACY_updateCollisions()
{
    foot_contacts_map.clear();
    _onGround=false;
    foot_sl_contacts_map.clear();
    //contactedWarp = NULL;
    //contactedWithWarp=false;
    //climbable_map.clear();
    environments_map.clear();
    contacted_bgos.clear();
    contacted_npc.clear();
    contacted_blocks.clear();
    contacted_players.clear();

    LEGACY_collided_top.clear();
    LEGACY_collided_left.clear();
    LEGACY_collided_right.clear();
    LEGACY_collided_bottom.clear();
    LEGACY_collided_center.clear();
    LEGACY_cliffDetected=false;

    LEGACY_collided_slope=false;
    LEGACY_collided_slope_angle_ratio=0.0f;
    LEGACY_collided_slope_celling=false;
    LEGACY_collided_slope_angle_ratio_celling=0.0f;

    #ifdef COLLIDE_DEBUG
    qDebug() << "=====Collision check and resolve begin======";
    #endif

    PGE_Phys_Object::updateCollisions();

    bool resolveBottom=false;
    bool resolveTop=false;
    bool resolveLeft=false;
    bool resolveRight=false;

    double backupX  = m_posRect.x();
    double backupY  = m_posRect.y();
    double _wallX   = m_posRect.x();
    double _floorY  = m_posRect.y();

    double _floorX_vel=0.0;//velocities sum
    double _floorX_num=0.0;//num of velocities
    double _floorY_vel=0.0;//velocities sum
    double _floorY_num=0.0;//num of velocities

    QVector<PGE_Phys_Object*> topbottom_blocks;
    QVector<PGE_Phys_Object*> floor_blocks;
    QVector<PGE_Phys_Object*> wall_blocks;

    if(!LEGACY_collided_bottom.isEmpty())
    {
        for(PlayerColliders::iterator it=LEGACY_collided_bottom.begin(); it!=LEGACY_collided_bottom.end() ; it++)
        {
            PGE_Phys_Object *collided= *it;
            switch(collided->type)
            {
                case PGE_Phys_Object::LVLBlock:
                {
                    LVL_Block *blk= static_cast<LVL_Block*>(collided);
                    if(!blk) continue;
                    foot_contacts_map[(intptr_t)collided]=(intptr_t)collided;
                    if(blk->m_slippery_surface) foot_sl_contacts_map[(intptr_t)collided]=(intptr_t)collided;
                    //if(blk->setup->bounce) blocks_to_hit.push_back(blk);
                    floor_blocks.push_back(blk);
                    _floorY_vel+=blk->speedYsum();
                    _floorY_num+=1.0;
                    _floorX_vel+=blk->speedXsum();
                    _floorX_num+=1.0;
                } break;
                case PGE_Phys_Object::LVLNPC:
                {
                    LVL_Npc *npc= static_cast<LVL_Npc*>(collided);
                    if(!npc) break;
                    foot_contacts_map[(intptr_t)collided]=(intptr_t)collided;
                    if(npc->m_slippery_surface) foot_sl_contacts_map[(intptr_t)collided]=(intptr_t)collided;
                    //if(blk->setup->bounce) blocks_to_hit.push_back(blk);
                    floor_blocks.push_back(npc);
                    #ifndef DISABLE_OLD_SPEEDADD
                    if((npc->collide_npc==COLLISION_TOP)||(npc->collide_npc==COLLISION_ANY))
                        npc->collision_speed_add.push_back(this);
                    #endif
                    _floorY_vel+=npc->speedYsum();
                    _floorY_num+=1.0;
                    _floorX_vel+=npc->speedXsum();
                    _floorX_num+=1.0;
                }
                break;
                default:break;
            }
        }
        if(_floorX_num!=0.0) _floorX_vel=_floorX_vel/_floorX_num;
        if(_floorY_num!=0.0) _floorY_vel=_floorY_vel/_floorY_num;
        if(!foot_contacts_map.isEmpty())
        {
            if(!is_scenery)
            {
                #ifndef DISABLE_OLD_SPEEDADD
                _velocityX_add = _floorX_vel;
                _velocityY_add = _floorY_vel;
                #endif
            }
        }

        if(isFloor(floor_blocks, &LEGACY_cliffDetected))
        {
            PGE_Phys_Object*nearest = nearestBlockY(floor_blocks);
            if(nearest)
            {
                LVL_Block *blk= static_cast<LVL_Block*>(nearest);
                if(blk && (blk->LEGACY_shape!=LVL_Block::shape_rect))
                {
                    if(blk->LEGACY_shape==LVL_Block::shape_tr_right_bottom)
                    {
                        LEGACY_collided_slope=true; LEGACY_collided_slope_angle_ratio=blk->shape_slope_angle_ratio;
                        _floorY = nearest->m_posRect.bottom()-SL_HeightTopRight(*this, nearest);
                        if(_floorY<nearest->top()) _floorY=nearest->m_posRect.top();
                        else if(_floorY>nearest->bottom()) _floorY=nearest->m_posRect.bottom();
                    }
                    else
                    if(blk->LEGACY_shape==LVL_Block::shape_tr_left_bottom)
                    {
                        LEGACY_collided_slope=true; LEGACY_collided_slope_angle_ratio=blk->shape_slope_angle_ratio;
                        _floorY = nearest->m_posRect.bottom()-SL_HeightTopLeft(*this, nearest);
                        if(_floorY<nearest->top()) _floorY=nearest->m_posRect.top();
                        else if(_floorY>nearest->bottom()) _floorY=nearest->m_posRect.bottom();
                    }
                    else
                        _floorY = nearest->m_posRect.top();
                    _floorY-=m_posRect.height();
                } else {
                    _floorY = nearest->m_posRect.top()-m_posRect.height();
                }
                resolveBottom=true;
            }
        }
        else
        {
            foot_contacts_map.clear();
            foot_sl_contacts_map.clear();
        }
    }

    if(!LEGACY_collided_top.isEmpty())
    {
        //blocks_to_hit.clear();
        for(PlayerColliders::iterator it=LEGACY_collided_top.begin(); it!=LEGACY_collided_top.end() ; it++)
        {
            PGE_Phys_Object *collided= *it;
            if(!collided) continue;
            switch(collided->type)
            {
            case PGE_Phys_Object::LVLPlayer:
                {
                    if((LEGACY_collide_player==COLLISION_ANY)||(LEGACY_collide_player==COLLISION_TOP))
                    {
                        if(!collision_speed_add.contains(collided))
                            collision_speed_add.push_back(collided);
                    }
                    continue;
                }
            break;
            case PGE_Phys_Object::LVLNPC:
                {
                    LVL_Npc *npc= static_cast<LVL_Npc*>(collided);
                    if(!npc) continue;
                    if(!npc->is_scenery && !npc->disableBlockCollision &&
                            ((LEGACY_collide_npc==COLLISION_ANY)||(LEGACY_collide_npc==COLLISION_TOP))
                            )
                    {
                        if(!collision_speed_add.contains(collided))
                            collision_speed_add.push_back(collided);
                        continue;
                    }
                }
            break;
            default:break;
            }
            topbottom_blocks.push_back(collided);
        }
        if(isFloor(topbottom_blocks))
        {
            PGE_Phys_Object*nearest = nearestBlockY(topbottom_blocks);
            if(nearest)
            {
                if(!resolveBottom) _floorY = nearest->m_posRect.bottom()+1;
                resolveTop=true;
            }
        }

        foreach(PGE_Phys_Object* x, floor_blocks)
            topbottom_blocks.push_back(x);
    }

    bool wall=false;
    if(!LEGACY_collided_left.isEmpty())
    {
        for(PlayerColliders::iterator it=LEGACY_collided_left.begin(); it!=LEGACY_collided_left.end() ; it++)
        {
            PGE_Phys_Object *collided= *it;
            if(collided) wall_blocks.push_back(collided);
        }
        if(isWall(wall_blocks))
        {
            PGE_Phys_Object*nearest = nearestBlock(wall_blocks);
            if(nearest)
            {
                _wallX = nearest->m_posRect.right();
                resolveLeft=true;
                wall=true;
            }
        }
    }

    if(!LEGACY_collided_right.isEmpty())
    {
        wall_blocks.clear();
        for(PlayerColliders::iterator it=LEGACY_collided_right.begin(); it!=LEGACY_collided_right.end() ; it++)
        {
            PGE_Phys_Object *collided= *it;
            if(collided) wall_blocks.push_back(collided);
        }
        if(isWall(wall_blocks))
        {
            PGE_Phys_Object*nearest = nearestBlock(wall_blocks);
            if(nearest)
            {
                _wallX = nearest->m_posRect.left()-m_posRect.width();
                resolveRight=true;
                wall=true;
            }
        }
    }

    if((resolveLeft||resolveRight) && (resolveTop||resolveBottom))
    {
        //check if on floor or in air
        bool _iswall=false;
        bool _isfloor=false;
        m_posRect.setX(_wallX);
        _isfloor = isFloor(floor_blocks, &LEGACY_cliffDetected);
        m_posRect.setPos(backupX, _floorY);
        _iswall = isWall(wall_blocks);
        m_posRect.setX(backupX);
        if(!_iswall && _isfloor)
        {
            resolveLeft = false;
            resolveRight = false;
        }
        if(!_isfloor && _iswall)
        {
            resolveTop = false;
            resolveBottom = false;
        }
    }

    bool needCorrect = false;
    double correctX = 0.0;
    double correctY = 0.0;
    if(resolveLeft || resolveRight)
    {
        //posRect.setX(_wallX);
        correctX = _wallX - m_posRect.x();
        needCorrect = true;
        setSpeedX(0);
        LEGACY_m_velocityX_add=0;
    }
    if(resolveBottom || resolveTop)
    {
        //posRect.setY(_floorY);
        correctY = _floorY - m_posRect.y();
        needCorrect = true;
        //float bumpSpeed=speedY();
        //if(resolveTop)
            setSpeedY(0.0);
        //else
        //  setSpeedY(_floorY_vel);
        LEGACY_m_velocityY_add=0;
        //if(!blocks_to_hit.isEmpty())
        //{
        //    LVL_Block*nearest = nearestBlock(blocks_to_hit);
        //    if(nearest)
        //    {
        //        //long npcid=nearest->data.npc_id;
        //        if(resolveBottom) nearest->hit(LVL_Block::down); else nearest->hit();
        //        //if( nearest->setup->hitable || (npcid!=0) || (nearest->destroyed) || nearest->setup->bounce )
        //        //    bump(resolveBottom,
        //        //         (resolveBottom ? physics_cur.velocity_jump_bounce : bumpSpeed),
        //        //         physics_cur.jump_time_bounce);
        //    }
        //}
        //if(resolveTop && !bumpUp && !bumpDown )
        //    jumpTime=0;
    }
    else
    {
        m_posRect.setY(backupY);
    }
    _stucked = ( (!LEGACY_collided_center.isEmpty()) && (!LEGACY_collided_bottom.isEmpty()) && (!wall) );

    if(needCorrect)
    {
        applyCorrectionToSA_stack(correctX, correctY);
    }

    collision_speed_add.clear();
    #ifndef DISABLE_OLD_SPEEDADD
    for(int i=0;i<collision_speed_add.size();i++)
    {
        if(collision_speed_add[i]->_velocityX_add!=0.0f)
            collision_speed_add[i]->setSpeedY(speedYsum());
        else
            collision_speed_add[i]->setSpeed(collision_speed_add[i]->speedX()+speedXsum(), speedYsum());
    }
    #endif

    #ifdef COLLIDE_DEBUG
    qDebug() << "=====Collision check and resolve end======";
    #endif
}

void LVL_Npc::LEGACY_detectCollisions(PGE_Phys_Object *collided)
{
    if(!collided) return;

    switch(collided->type)
    {
        case PGE_Phys_Object::LVLBlock:
        {
            //Don't collide with blocks if disabled!
            if(disableBlockCollision) break;

            #ifdef COLLIDE_DEBUG
            qDebug() << "Player:"<<posRect.x()<<posRect.y()<<posRect.width()<<posRect.height();
            #endif
            LVL_Block *blk= static_cast<LVL_Block*>(collided);
            if(blk)
            {
                if(blk->destroyed)
                {
                    #ifdef COLLIDE_DEBUG
                    qDebug() << "Destroyed!";
                    #endif
                    break;
                }
            }
            else
            {
                #ifdef COLLIDE_DEBUG
                qDebug() << "Wrong cast";
                #endif
                break;
            }

            contacted_blocks[intptr_t(collided)]=collided;

            if( ((!forceCollideCenter)&&(!collided->m_posRect.collideRect(m_posRect)))||
                ((forceCollideCenter)&&(!collided->m_posRect.collideRectDeep(m_posRect, 1.0, -3.0))) )
            {
                #ifdef COLLIDE_DEBUG
                qDebug() << "No, is not collidng";
                #endif
                break;
            }
            #ifdef COLLIDE_DEBUG
            qDebug() << "Collided item! "<<collided->type<<" " <<collided->posRect.center().x()<<collided->posRect.center().y();
            #endif

            if((bumpUp||bumpDown)&&(!forceCollideCenter))
            {
                #ifdef COLLIDE_DEBUG
                qDebug() << "Bump? U'r dumb!";
                #endif
                break;
            }

//            PGE_PointF c1 = posRect.center();
//            PGE_RectF &r1 = posRect;
//            PGE_PointF cc = collided->posRect.center();
//            PGE_RectF  rc = collided->posRect;

            switch(collided->LEGACY_collide_npc)
            {
                case COLLISION_TOP:
                {
//                    PGE_RectF &r1=posRect;
//                    PGE_RectF  rc = collided->posRect;
//                    float summSpeedY=(speedY()+_velocityY_add)-(collided->speedY()+collided->_velocityY_add);
//                    float summSpeedYprv=_velocityY_prev-collided->_velocityY_prev;
                    if(isCollideFloorToponly(*this, collided))
                    {
                        if(blk->isHidden) break;
                        LEGACY_collided_bottom[intptr_t(collided)]=collided;//bottom of player
                        if(blk->setup->setup.lava) kill(DAMAGE_LAVABURN);
                        #ifdef COLLIDE_DEBUG
                        qDebug() << "Top of block";
                        #endif
                    }
                }
                break;
            case COLLISION_ANY:
            {
                #ifdef COLLIDE_DEBUG
                bool found=false;
                #endif
                //*****************************Feet of NPC****************************/
                if(
                    (( (blk->LEGACY_shape==LVL_Block::shape_rect)||
                       (blk->LEGACY_shape==LVL_Block::shape_tr_right_top)||
                       (blk->LEGACY_shape==LVL_Block::shape_tr_left_top) ) && isCollideFloor(*this, collided))||
                    ((blk->LEGACY_shape==LVL_Block::shape_tr_right_bottom)&&isCollideSlopeFloor(*this, collided, SLOPE_RIGHT)) ||
                    ((blk->LEGACY_shape==LVL_Block::shape_tr_left_bottom)&&isCollideSlopeFloor(*this, collided, SLOPE_LEFT))

                  ){
                    if(blk->isHidden) break;
                    LEGACY_collided_bottom[intptr_t(collided)]=collided;//bottom of NPC
                    if(blk->setup->setup.lava) kill(DAMAGE_LAVABURN);
                    //else if(blk->setup->danger==2||blk->setup->danger==-3||blk->setup->danger==4) harm(1);
                }
                //*****************************Head of NPC****************************/
                else if(
                        (( (blk->LEGACY_shape==LVL_Block::shape_rect)||
                            (blk->LEGACY_shape==LVL_Block::shape_tr_left_bottom)||
                            (blk->LEGACY_shape==LVL_Block::shape_tr_right_bottom)) &&
                            isCollideCelling(*this, collided, _heightDelta, forceCollideCenter))||
                        ((blk->LEGACY_shape==LVL_Block::shape_tr_left_top)&&isCollideSlopeCelling(*this, collided, SLOPE_RIGHT)) ||
                        ((blk->LEGACY_shape==LVL_Block::shape_tr_right_top)&&isCollideSlopeCelling(*this, collided, SLOPE_LEFT))
                       )
                {
                    LEGACY_collided_top[intptr_t(collided)]=collided;//top of NPC
                    if(blk->setup->setup.lava) kill(DAMAGE_LAVABURN);
                    //else if(blk->setup->danger==-2||blk->setup->danger==-3||blk->setup->danger==4) harm(1);
                }
                //*****************************Left****************************/
                else if( (isCollideLeft(*this, collided)&&(blk->LEGACY_shape==LVL_Block::shape_rect))||
                         (isCollideLeft(*this, collided)&&(blk->LEGACY_shape==LVL_Block::shape_tr_left_bottom)
                          &&(m_posRect.bottom()>=(collided->m_posRect.top()+SL_HeightTopRight(*this, collided)+1.0)))||
                         (isCollideLeft(*this, collided)&&(blk->LEGACY_shape==LVL_Block::shape_tr_right_top)
                          &&(m_posRect.top()<=(collided->m_posRect.bottom()-SL_HeightTopRight(*this, collided)-1.0))) )
                {
                    if(blk->isHidden) break;
                    LEGACY_collided_left[intptr_t(collided)]=collided;//right of NPC
                    if(blk->setup->setup.lava) kill(DAMAGE_LAVABURN);
                    //else if(blk->setup->danger==-1||blk->setup->danger==3||blk->setup->danger==4) harm(1);
                }
                //*****************************Right****************************/
                else if( (isCollideRight(*this, collided)&&(blk->LEGACY_shape==LVL_Block::shape_rect))||
                         (isCollideRight(*this, collided)&&(blk->LEGACY_shape==LVL_Block::shape_tr_right_bottom)
                         &&(m_posRect.bottom()>=(collided->m_posRect.top()+SL_HeightTopLeft(*this, collided)+1.0)))||
                         (isCollideRight(*this, collided)&&(blk->LEGACY_shape==LVL_Block::shape_tr_left_top)
                         &&(m_posRect.top()<=(collided->m_posRect.bottom()-SL_HeightTopLeft(*this, collided)-1.0)))
                       )
                {
                    if(blk->isHidden) break;
                    LEGACY_collided_right[intptr_t(collided)]=collided;//left of NPC
                    if(blk->setup->setup.lava) kill(DAMAGE_LAVABURN);
                    //else if(blk->setup->danger==1||blk->setup->danger==3||blk->setup->danger==4) harm(1);
                }


                float c=forceCollideCenter? 0.0f : 1.0f;
                //*****************************Center****************************/
                if( ((!forceCollideCenter && blk->LEGACY_shape==LVL_Block::shape_rect)||(forceCollideCenter))
                        && blk->m_posRect.collideRectDeep(m_posRect,
                                                 fabs(LEGACY_m_velocityX_prev+LEGACY_m_velocityX_add)*c+c*2.0,
                                                 fabs(LEGACY_m_velocityY_prev+LEGACY_m_velocityY_add)*c+c*2.0)
                        )
                {
                    if(blk->isHidden && !forceCollideCenter) break;
                    LEGACY_collided_center[intptr_t(collided)]=collided;
                }
                break;
            }
            default: break;
            }
            break;
        }
        case PGE_Phys_Object::LVLBGO:
        {
            LVL_Bgo *bgo= static_cast<LVL_Bgo*>(collided);
            if(bgo)
            {
                contacted_bgos[intptr_t(collided)]=collided;
            }
            break;
        }
        case PGE_Phys_Object::LVLNPC:
        {
            LVL_Npc *npc= static_cast<LVL_Npc*>(collided);
            if(npc)
            {
                if(npc->killed)        break;
                if(npc->data.friendly) break;
                if(npc->isGenerator) break;
            }

            if( ((!forceCollideCenter)&&(!collided->m_posRect.collideRect(m_posRect)))||
                ((forceCollideCenter)&&(!collided->m_posRect.collideRectDeep(m_posRect, 1.0, -3.0))) )
            {
                break;
            }

            if(!npc->isActivated) break;

            contacted_npc[intptr_t(collided)]=collided;

            if(isGenerator) break;
            if(disableNpcCollision) break;
//            PGE_PointF c1 = posRect.center();
//            PGE_RectF &r1 = posRect;
//            PGE_PointF cc = collided->posRect.center();
//            PGE_RectF  rc = collided->posRect;

            if(disableBlockCollision) break;

            switch(collided->LEGACY_collide_npc)
            {
                case COLLISION_TOP:
                {
//                    PGE_RectF &r1=posRect;
//                    PGE_RectF  rc = collided->posRect;
                    if(isCollideFloorToponly(*this, collided))
//                            (
//                                (speedY() >= 0.0)
//                                &&
//                                (r1.bottom() < rc.top()+_velocityY_prev+collided->_velocityY_prev)
//                                &&
//                                (
//                                     (r1.left()<rc.right()-1 ) &&
//                                     (r1.right()>rc.left()+1 )
//                                 )
//                             )
//                            ||
//                            (r1.bottom() <= rc.top())
//                            )
                    {
                        LEGACY_collided_bottom[intptr_t(collided)] = collided;//bottom of player
                        #ifdef COLLIDE_DEBUG
                        qDebug() << "Top of block";
                        #endif
                    }
                }
                break;
                case COLLISION_ANY:
                {
                    #ifdef COLLIDE_DEBUG
                    bool found=false;
                    #endif
//                    double xSpeed = Maths::max(fabs(speedXsum()), fabs(_velocityX_prev)) * Maths::sgn(speedXsum());
//                    double ySpeed = Maths::max(fabs(speedYsum()), fabs(_velocityY_prev)) * Maths::sgn(speedYsum());
//                    double xSpeedO = Maths::max(fabs(collided->speedXsum()), fabs(collided->_velocityX_prev)) * Maths::sgn(collided->speedXsum());
//                    double ySpeedO = Maths::max(fabs(collided->speedYsum()), fabs(collided->_velocityY_prev)) * Maths::sgn(collided->speedYsum());
                    //*****************************Feet of NPC****************************/
                    if(isCollideFloor(*this, collided))
//                            (
//                                (fabs(speedYsum()) >= 0.0)
//                                &&
//                                (floor(r1.bottom()) < rc.top()+ySpeed+ySpeedO)
//                                &&( !( (r1.left()>=rc.right()-0.2) || (r1.right() <= rc.left()+0.2) ) )
//                             )
//                            ||
//                            (r1.bottom() <= rc.top())
                    {
                            LEGACY_collided_bottom[intptr_t(collided)]=collided;//bottom of player
                            #ifdef COLLIDE_DEBUG
                            qDebug() << "Top of block";
                            found=true;
                            #endif
                    }
                    //*****************************Head of NPC****************************/
                    else if(isCollideCelling(*this, collided, _heightDelta, forceCollideCenter))
//                    else if( (
//                                 (  ((!forceCollideCenter)&&(-fabs(speedYsum())<0.0))||(forceCollideCenter&&(-fabs(speedYsum())<=0.0))   )
//                                 &&
//                                 (r1.top() > rc.bottom()+ySpeed-1.0+ySpeedO+_heightDelta)
//                                 &&( !( (r1.left()>=rc.right()-0.5 ) || (r1.right() <= rc.left()+0.5 ) ) )
//                              )
//                             )
                    {
                        LEGACY_collided_top[intptr_t(collided)]=collided;//top of player
                        #ifdef COLLIDE_DEBUG
                        qDebug() << "Bottom of block";
                        found=true;
                        #endif
                    }
                    //*****************************Left****************************/
                    else if( /*(speedXsum()<0.0) && (c1.x() > cc.x()) && (r1.left() >= rc.right()+xSpeed+xSpeedO-1.0)
                             && ( (r1.top()<rc.bottom())&&(r1.bottom()>rc.top()) )*/ isCollideLeft(*this, collided) )
                    {
                        LEGACY_collided_left[intptr_t(collided)]=collided;//right of player
                        #ifdef COLLIDE_DEBUG
                        qDebug() << "Right of block";
                        #endif
                    }
                    //*****************************Right****************************/
                    else if( /*(speedX()>0.0) && (c1.x() < cc.x()) && ( r1.right() <= rc.left()+xSpeed+xSpeedO+1.0)
                             && ( (r1.top()<rc.bottom())&&(r1.bottom()>rc.top()) )*/ isCollideRight(*this, collided) )
                    {
                        LEGACY_collided_right[intptr_t(collided)]=collided;//left of player
                        #ifdef COLLIDE_DEBUG
                        qDebug() << "Left of block";
                        found=true;
                        #endif
                    }


                    float c=forceCollideCenter? 0.0f : 1.0f;
                    //*****************************Center****************************/
                    #ifdef COLLIDE_DEBUG
                    qDebug() << "block" <<posRect.top()<<":"<<blk->posRect.bottom()
                             << "block" <<posRect.bottom()<<":"<<blk->posRect.top()<<" collide?"<<
                                blk->posRect.collideRectDeep(posRect,
                                                                                     fabs(_velocityX_prev)*c+c*2.0,
                                                                                     fabs(_velocityY_prev)*c+c*2.0) <<
                                "depths: "<< fabs(_velocityX_prev)*c+c*2.0 <<
                            fabs(_velocityY_prev)*c+c;
                    #endif
                    if( npc->m_posRect.collideRectDeep(m_posRect,
                                                     fabs(LEGACY_m_velocityX_prev)*c+c*2.0,
                                                     fabs(LEGACY_m_velocityY_prev)*c+c*2.0)
                            )
                    {
                        if(!forceCollideCenter) break;
                        LEGACY_collided_center[intptr_t(collided)]=collided;
                        #ifdef COLLIDE_DEBUG
                        qDebug() << "Center of block";
                        found=true;
                        #endif
                    }

                    #ifdef COLLIDE_DEBUG
                    qDebug() << "---checked---" << (found?"and found!": "but nothing..." )<<
                                r1.left()<< "<="<< rc.right()<<"+"<<xSpeed ;
                    #endif
                    break;
                }
            default: break;
            }

            break;
        }
        case PGE_Phys_Object::LVLPhysEnv:
        {
            LVL_PhysEnv *env= static_cast<LVL_PhysEnv*>(collided);
            if(env)
            {
                if(env) environments_map[intptr_t(env)]=env->env_type;
            }
            break;
        }
    case PGE_Phys_Object::LVLPlayer:
        {
            contacted_players[intptr_t(collided)]=collided;

            //PGE_PointF c1 = posRect.center();
            PGE_RectF &r1 = m_posRect;
            //PGE_PointF cc = collided->posRect.center();
            PGE_RectF  rc = collided->m_posRect;

            //double xSpeed = Maths::max(fabs(speedX()), fabs(_velocityX_prev)) * Maths::sgn(speedX());
            double ySpeed = Maths::max(fabs(speedY()), fabs(LEGACY_m_velocityY_prev)) * Maths::sgn(speedY());
            //double xSpeedO = Maths::max(fabs(collided->speedX()), fabs(collided->_velocityX_prev)) * Maths::sgn(collided->speedX());
            double ySpeedO = Maths::max(fabs(collided->speedY()), fabs(collided->LEGACY_m_velocityY_prev)) * Maths::sgn(collided->speedY());
            if( (
                         (  ((!forceCollideCenter)&&(speedY()<0.0))||(forceCollideCenter&&(speedY()<=0.0))   )
                         &&
                         (r1.top() > rc.bottom()+ySpeed-1.0+ySpeedO+_heightDelta)
                         &&( !( (r1.left()>=rc.right()-0.5 ) || (r1.right() <= rc.left()+0.5 ) ) )
                      )
                     )
            {
                LEGACY_collided_top[intptr_t(collided)] = collided;//top of player
            }
            break;
        }
    default: break;
    }
}
#endif

bool LVL_Npc::onCliff()
{
    return m_cliff;
}

bool LVL_Npc::onGround()
{
    return m_stand;
}

#ifdef OLD_COLLIDERS
void LVL_Npc::updateSpeedAddingStack()
{
    if(!LEGACY_collided_bottom.isEmpty())
    {
        double _floorX_vel=0.0;//velocities sum
        double _floorX_num=0.0;//num of velocities
        double _floorY_vel=0.0;//velocities sum
        double _floorY_num=0.0;//num of velocities
        for(PlayerColliders::iterator it = LEGACY_collided_bottom.begin(); it != LEGACY_collided_bottom.end() ; it++)
        {
            PGE_Phys_Object *collided= *it;
            switch(collided->type)
            {
                case PGE_Phys_Object::LVLBlock:
                {
                    LVL_Block *blk= static_cast<LVL_Block*>(collided);
                    if(!blk) continue;
                    _floorY_vel += blk->speedYsum();
                    _floorY_num += 1.0;
                    _floorX_vel += blk->speedXsum();
                    _floorX_num += 1.0;
                    if((blk->speedXsum() != 0.0)||(blk->speedYsum() != 0.0))
                    {
                        if(!blk->m_speedAddingTopElements.contains(this))
                            blk->m_speedAddingTopElements.append(this);
                        if(!m_speedAddingBottomElements.contains(collided))
                            m_speedAddingBottomElements.append(collided);
                    }
                }
                break;
                case PGE_Phys_Object::LVLNPC:
                {
                    LVL_Npc *npc= static_cast<LVL_Npc*>(collided);
                    if(!npc) continue;
                    _floorY_vel+=npc->speedYsum();
                    _floorY_num+=1.0;
                    _floorX_vel+=npc->speedXsum();
                    _floorX_num+=1.0;
                    //if(is_scenery)
                    //{
                    if(!npc->m_speedAddingTopElements.contains(this))
                        npc->m_speedAddingTopElements.append(this);
                    if(!m_speedAddingBottomElements.contains(collided))
                        m_speedAddingBottomElements.append(collided);
                    //}
                }
                break;
                default:break;
            }
        }
        if(_floorX_num != 0.0) _floorX_vel = _floorX_vel/_floorX_num;
        if(_floorY_num != 0.0) _floorY_vel = _floorY_vel/_floorY_num;
        #ifndef DISABLE_OLD_SPEEDADD
        if(!foot_contacts_map.isEmpty())
        {
            if(!is_scenery)
            {
                _velocityX_add = _floorX_vel;
                _velocityY_add = _floorY_vel;
            }
        }
        #endif
    }
    for(int i=0; i<collision_speed_add.size(); i++)
        collision_speed_add[i]->updateSpeedAddingStack();
}

void LVL_Npc::applyCorrectionToSA_stack(double offsetX, double offsetY)
{
    m_posRect.setPos(m_posRect.x()+offsetX, m_posRect.y()+offsetY);
    _syncPosition();
    for(int i=0; i<collision_speed_add.size(); i++)
        collision_speed_add[i]->applyCorrectionToSA_stack(offsetX, offsetY);
}
#endif

//void LVL_Npc::iterateSpeedAddingStack(double offsetX, double offsetY)
//{
//    for(int i=0; i<m_speedAddingTopElements.size(); i++)
//    {
//        if( !m_speedAddingTopElements[i] || !this->posRect.collideRect( m_speedAddingTopElements[i]->posRect ) )
//        {
//            m_speedAddingTopElements.removeAt(i); i--;
//            continue;
//        }
//        PGE_RectF &posR = m_speedAddingTopElements[i]->posRect;
//        m_speedAddingTopElements[i]->setPos(posR.x()+offsetX, posR.y()+offsetY);
//        m_speedAddingTopElements[i]->iterateSpeedAddingStack(offsetX, offsetY);
//    }
//}
