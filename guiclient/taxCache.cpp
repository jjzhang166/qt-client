/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "taxCache.h"

#include <QString>

void taxCache::clear()
{
  for (unsigned h = Pct; h <= Amount; h++)
    for (unsigned i = A; i <= C; i++)
      for (unsigned j = Line; j <= Adj; j++)
	cache[h][i][j] = 0.0;

  for (unsigned i = Tax; i <= Type; i++)
    for (unsigned j = Line; j <= Adj; j++)
      ids[i][j] = 0;
}

taxCache::taxCache()
{
  clear();
}

taxCache::taxCache(taxCache& p)
{
  for (unsigned h = Pct; h <= Amount; h++)
    for (unsigned i = A; i <= C; i++)
      for (unsigned j = Line; j <= Adj; j++)
	cache[h][i][j] = p.cache[h][i][j];

  for (unsigned i = Tax; i <= Type; i++)
    for (unsigned j = Line; j <= Adj; j++)
      ids[i][j] = p.ids[i][j];
}

taxCache::~taxCache()
{
}

double	taxCache::adj() const
{
  return adj(0) + adj(1) + adj(2);
}

double	taxCache::adj(unsigned p) const
{
  return cache[Amount][p][Adj];
}

int	taxCache::adjId() const
{
  return ids[Tax][Adj];
}

int	taxCache::adjType() const
{
  return ids[Type][Adj];
}

double	taxCache::adjPct() const
{
  return adjPct(0) + adjPct(1) + adjPct(2);
}

double	taxCache::adjPct(unsigned p) const
{
  return cache[Pct][p][Adj];
}


double	taxCache::freight() const
{
  return freight(0) + freight(1) + freight(2);
}

double	taxCache::freight(unsigned p) const
{
  return cache[Amount][p][Freight];
}

int	taxCache::freightId() const
{
  return ids[Tax][Freight];
}

int	taxCache::freightType() const
{
  return ids[Type][Freight];
}

double	taxCache::freightPct() const
{
  return freightPct(0) + freightPct(1) + freightPct(2);
}

double taxCache::freightPct(unsigned p) const
{
  return cache[Pct][p][Freight];
}


double	taxCache::line() const
{
  return line(0) + line(1) + line(2);
}

double	taxCache::line(unsigned p) const
{
  return cache[Amount][p][Line];
}

double	taxCache::linePct() const
{
  return linePct(0) + linePct(1) + linePct(2);
}

double	taxCache::linePct(unsigned p) const
{
  return cache[Pct][p][Line];
}


void	taxCache::setAdj(const double a, const double b, const double c)
{
  cache[Amount][A][Adj] = a;
  cache[Amount][B][Adj] = b;
  cache[Amount][C][Adj] = c;
}

void	taxCache::setAdjPct(const double a, const double b, const double c)
{
  cache[Pct][A][Adj] = a;
  cache[Pct][B][Adj] = b;
  cache[Pct][C][Adj] = c;
}

void	taxCache::setAdjId(int p)
{
  ids[Tax][Adj] = p;
}

void	taxCache::setAdjType(int p)
{
  ids[Type][Adj] = p;
}

void	taxCache::setFreight(const double a, const double b, const double c)
{
  cache[Amount][A][Freight] = a;
  cache[Amount][B][Freight] = b;
  cache[Amount][C][Freight] = c;
}

void	taxCache::setFreightPct(const double a, const double b, const double c)
{
  cache[Pct][A][Freight] = a;
  cache[Pct][B][Freight] = b;
  cache[Pct][C][Freight] = c;
}

void	taxCache::setFreightId(int p)
{
  ids[Tax][Freight] = p;
}

void	taxCache::setFreightType(int p)
{
  ids[Type][Freight] = p;
}

void	taxCache::setLine(const double a, const double b, const double c)
{
  cache[Amount][A][Line] = a;
  cache[Amount][B][Line] = b;
  cache[Amount][C][Line] = c;
}

void	taxCache::setLinePct(const double a, const double b, const double c)
{
  cache[Pct][A][Line] = a;
  cache[Pct][B][Line] = b;
  cache[Pct][C][Line] = c;
}


double	taxCache::total() const
{
  return line() + freight() + adj();
}

double	taxCache::total(unsigned p) const
{
  return line(p) + freight(p) + adj(p);
}

QString	taxCache::toString() const
{
  QString result = "";
  QString id[]   =	{ "Tax", "Type" };
  QString info[] =	{ "Pct", "Amount" };
  QString rate[] =	{ "A", "B", "C" };
  QString part[] =	{ "Line", "Freight", "Adj" };

  for (unsigned h = Pct; h <= Amount; h++)
  {
    for (unsigned i = A; i <= C; i++)
    {
      result += info[h] + " " + rate[i] + ":";
      for (unsigned j = Line; j <= Adj; j++)
	result += "\t" + part[j] + " = " + QString::number(cache[h][i][j]);
      result += "\n";
    }
    result += "\n";
  }

  for (unsigned i = Tax; i <= Type; i++)
  {
    result += id[i] + ":";
    for (unsigned j = Line; j <= Adj; j++)
      result += "\t" + part[j] + " = " + QString::number(ids[i][j]);
    result += "\n";
  }

  return result;
}
