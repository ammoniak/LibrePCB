/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * https://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBREPCB_CORE_BI_POLYGON_H
#define LIBREPCB_CORE_BI_POLYGON_H

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "../../../graphics/graphicslayername.h"
#include "../../../serialization/serializableobject.h"
#include "../../../types/length.h"
#include "../../../types/point.h"
#include "../../../types/uuid.h"
#include "bi_base.h"

#include <QtCore>

/*******************************************************************************
 *  Namespace / Forward Declarations
 ******************************************************************************/
namespace librepcb {

class BGI_Polygon;
class Board;
class Path;
class Polygon;
class PolygonGraphicsItem;
class Project;

/*******************************************************************************
 *  Class BI_Polygon
 ******************************************************************************/

/**
 * @brief The BI_Polygon class
 */
class BI_Polygon final : public BI_Base, public SerializableObject {
  Q_OBJECT

public:
  // Constructors / Destructor
  BI_Polygon() = delete;
  BI_Polygon(const BI_Polygon& other) = delete;
  BI_Polygon(Board& board, const BI_Polygon& other);
  BI_Polygon(Board& board, const SExpression& node, const Version& fileFormat);
  BI_Polygon(Board& board, const Polygon& polygon);
  BI_Polygon(Board& board, const Uuid& uuid, const GraphicsLayerName& layerName,
             const UnsignedLength& lineWidth, bool fill, bool isGrabArea,
             const Path& path);
  ~BI_Polygon() noexcept;

  // Getters
  Polygon& getPolygon() noexcept { return *mPolygon; }
  const Polygon& getPolygon() const noexcept { return *mPolygon; }
  const Uuid& getUuid() const
      noexcept;  // convenience function, e.g. for template usage
  PolygonGraphicsItem& getGraphicsItem() noexcept { return *mGraphicsItem; }
  bool isSelectable() const noexcept override;

  // General Methods
  void addToBoard() override;
  void removeFromBoard() override;

  /// @copydoc ::librepcb::SerializableObject::serialize()
  void serialize(SExpression& root) const override;

  // Inherited from BI_Base
  Type_t getType() const noexcept override { return BI_Base::Type_t::Polygon; }
  QPainterPath getGrabAreaScenePx() const noexcept override;
  void setSelected(bool selected) noexcept override;

  // Operator Overloadings
  BI_Polygon& operator=(const BI_Polygon& rhs) = delete;

private slots:

  void boardAttributesChanged();

private:
  void init();

  // General
  QScopedPointer<Polygon> mPolygon;
  QScopedPointer<PolygonGraphicsItem> mGraphicsItem;
};

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb

#endif
