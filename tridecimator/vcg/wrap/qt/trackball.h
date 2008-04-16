/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2007                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

$Log: not supported by cvs2svn $

****************************************************************************/

#ifndef QT_TRACKBALL_H
#define QT_TRACKBALL_H

/// Takes a QT MouseButton, some QT KeyboardModifiers and returns the equivalent Trackball::Button
static vcg::Trackball::Button QT2VCG (Qt::MouseButton qtbt, Qt::KeyboardModifiers modifiers)
{
  int vcgbt = vcg::Trackball::BUTTON_NONE;

  if (qtbt & Qt::LeftButton)	vcgbt |= vcg::Trackball::BUTTON_LEFT;
  if (qtbt & Qt::RightButton)	vcgbt |= vcg::Trackball::BUTTON_RIGHT;
  if (qtbt & Qt::MidButton)		vcgbt |= vcg::Trackball::BUTTON_MIDDLE;

  if (modifiers & Qt::ShiftModifier)		vcgbt |= vcg::Trackball::KEY_SHIFT;
  if (modifiers & Qt::ControlModifier)	vcgbt |= vcg::Trackball::KEY_CTRL;
  if (modifiers & Qt::AltModifier)			vcgbt |= vcg::Trackball::KEY_ALT;

  return vcg::Trackball::Button (vcgbt);
}

/// Takes some QT KeyboardModifiers and returns the mouse wheel related Trackball::Button
static vcg::Trackball::Button QTWheel2VCG (Qt::KeyboardModifiers modifiers)
{
  int vcgbt = vcg::Trackball::WHEEL;

  if (modifiers & Qt::ShiftModifier)		vcgbt |= vcg::Trackball::KEY_SHIFT;
  if (modifiers & Qt::ControlModifier)	vcgbt |= vcg::Trackball::KEY_CTRL;
  if (modifiers & Qt::AltModifier)			vcgbt |= vcg::Trackball::KEY_ALT;

  return vcg::Trackball::Button (vcgbt);
}

#endif // QT_TRACKBALL_H