/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTSETUP_H__
#define __QTSETUP_H__

#include <Qt>
#include <QtScript>
#include <QTextDocument>

void setupQt(QScriptEngine *engine);

Q_DECLARE_METATYPE(Qt::MatchFlags);
Q_DECLARE_METATYPE(enum Qt::AlignmentFlag);
Q_DECLARE_METATYPE(enum Qt::ApplicationAttribute);
Q_DECLARE_METATYPE(enum Qt::ArrowType);
Q_DECLARE_METATYPE(enum Qt::AspectRatioMode);
Q_DECLARE_METATYPE(enum Qt::Axis);
Q_DECLARE_METATYPE(enum Qt::BGMode);
Q_DECLARE_METATYPE(enum Qt::BrushStyle);
Q_DECLARE_METATYPE(enum Qt::CaseSensitivity);
Q_DECLARE_METATYPE(enum Qt::CheckState);
Q_DECLARE_METATYPE(enum Qt::ClipOperation);
Q_DECLARE_METATYPE(enum Qt::ConnectionType);
Q_DECLARE_METATYPE(enum Qt::ContextMenuPolicy);
Q_DECLARE_METATYPE(enum Qt::Corner);
Q_DECLARE_METATYPE(enum Qt::CursorShape);
Q_DECLARE_METATYPE(enum Qt::DateFormat);
Q_DECLARE_METATYPE(enum Qt::DayOfWeek);
Q_DECLARE_METATYPE(enum Qt::DockWidgetArea);
Q_DECLARE_METATYPE(enum Qt::DropAction);
Q_DECLARE_METATYPE(enum Qt::EventPriority);
Q_DECLARE_METATYPE(enum Qt::FillRule);
Q_DECLARE_METATYPE(enum Qt::FindChildOption);
Q_DECLARE_METATYPE(Qt::FindChildOptions);
Q_DECLARE_METATYPE(enum Qt::FocusReason);
Q_DECLARE_METATYPE(enum Qt::GlobalColor);
Q_DECLARE_METATYPE(enum Qt::HitTestAccuracy);
Q_DECLARE_METATYPE(enum Qt::ImageConversionFlag);
Q_DECLARE_METATYPE(enum Qt::InputMethodQuery);
Q_DECLARE_METATYPE(enum Qt::ItemDataRole);
Q_DECLARE_METATYPE(enum Qt::ItemFlag);
Q_DECLARE_METATYPE(enum Qt::ItemSelectionMode);
Q_DECLARE_METATYPE(enum Qt::Key);
Q_DECLARE_METATYPE(enum Qt::KeyboardModifier);
Q_DECLARE_METATYPE(enum Qt::LayoutDirection);
Q_DECLARE_METATYPE(enum Qt::MaskMode);
Q_DECLARE_METATYPE(enum Qt::MatchFlag);
Q_DECLARE_METATYPE(enum Qt::Modifier);
Q_DECLARE_METATYPE(enum Qt::MouseButton);
Q_DECLARE_METATYPE(enum Qt::Orientation);
Q_DECLARE_METATYPE(enum Qt::PenCapStyle);
Q_DECLARE_METATYPE(enum Qt::PenJoinStyle);
Q_DECLARE_METATYPE(enum Qt::PenStyle);
Q_DECLARE_METATYPE(enum Qt::ScrollBarPolicy);
Q_DECLARE_METATYPE(enum Qt::ShortcutContext);
Q_DECLARE_METATYPE(enum Qt::SizeHint);
Q_DECLARE_METATYPE(enum Qt::SizeMode);
Q_DECLARE_METATYPE(enum Qt::SortOrder);
Q_DECLARE_METATYPE(enum Qt::TextElideMode);
Q_DECLARE_METATYPE(enum Qt::TextFlag);
Q_DECLARE_METATYPE(enum Qt::TextFormat);
Q_DECLARE_METATYPE(enum Qt::TextInteractionFlag);
Q_DECLARE_METATYPE(enum Qt::TimeSpec);
Q_DECLARE_METATYPE(enum Qt::ToolBarArea);
Q_DECLARE_METATYPE(enum Qt::ToolButtonStyle);
Q_DECLARE_METATYPE(enum Qt::TransformationMode);
Q_DECLARE_METATYPE(enum Qt::UIEffect);
Q_DECLARE_METATYPE(enum Qt::WhiteSpaceMode);
Q_DECLARE_METATYPE(enum Qt::WidgetAttribute);
Q_DECLARE_METATYPE(enum Qt::WindowFrameSection);
Q_DECLARE_METATYPE(enum Qt::WindowModality);
Q_DECLARE_METATYPE(enum Qt::WindowState);
Q_DECLARE_METATYPE(enum Qt::WindowType);

#endif
