/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "StructureScanComplexItem.h"
#include "JsonHelper.h"
#include "MissionController.h"
#include "QGCGeo.h"
#include "QGroundControlQmlGlobal.h"
#include "QGCQGeoCoordinate.h"
#include "SettingsManager.h"
#include "AppSettings.h"
#include "QGCQGeoCoordinate.h"

#include <QPolygonF>

QGC_LOGGING_CATEGORY(StructureScanComplexItemLog, "StructureScanComplexItemLog")

const char* StructureScanComplexItem::settingsGroup =               "StructureScan";
const char* StructureScanComplexItem::_entranceAltName =            "EntranceAltitude";
const char* StructureScanComplexItem::scanBottomAltName =           "ScanBottomAlt";
const char* StructureScanComplexItem::structureHeightName =         "StructureHeight";
const char* StructureScanComplexItem::layersName =                  "Layers";
const char* StructureScanComplexItem::gimbalPitchName =             "GimbalPitch";
const char* StructureScanComplexItem::startFromTopName =            "StartFromTop";

const char* StructureScanComplexItem::jsonComplexItemTypeValue =    "StructureScan";
const char* StructureScanComplexItem::_jsonCameraCalcKey =          "CameraCalc";

StructureScanComplexItem::StructureScanComplexItem(Vehicle* vehicle, bool flyView, const QString& kmlOrShpFile, QObject* parent)
    : ComplexMissionItem        (vehicle, flyView, parent)
    , _metaDataMap              (FactMetaData::createMapFromJsonFile(QStringLiteral(":/json/StructureScan.SettingsGroup.json"), this /* QObject parent */))
    , _sequenceNumber           (0)
    , _entryVertex              (0)
    , _ignoreRecalc             (false)
    , _scanDistance             (0.0)
    , _cameraShots              (0)
    , _cameraCalc               (vehicle, settingsGroup)
    , _scanBottomAltFact        (settingsGroup, _metaDataMap[scanBottomAltName])
    , _structureHeightFact      (settingsGroup, _metaDataMap[structureHeightName])
    , _layersFact               (settingsGroup, _metaDataMap[layersName])
    , _gimbalPitchFact          (settingsGroup, _metaDataMap[gimbalPitchName])
    , _startFromTopFact         (settingsGroup, _metaDataMap[startFromTopName])
    , _entranceAltFact          (settingsGroup, _metaDataMap[_entranceAltName])
{
    _editorQml = "qrc:/qml/StructureScanEditor.qml";

    _entranceAltFact.setRawValue(qgcApp()->toolbox()->settingsManager()->appSettings()->defaultMissionItemAltitude()->rawValue());

    connect(&_entranceAltFact,      &Fact::valueChanged, this, &StructureScanComplexItem::_setDirty);
    connect(&_scanBottomAltFact,    &Fact::valueChanged, this, &StructureScanComplexItem::_setDirty);
    connect(&_layersFact,           &Fact::valueChanged, this, &StructureScanComplexItem::_setDirty);
    connect(&_gimbalPitchFact,      &Fact::valueChanged, this, &StructureScanComplexItem::_setDirty);
    connect(&_startFromTopFact,     &Fact::valueChanged, this, &StructureScanComplexItem::_setDirty);

    connect(&_startFromTopFact,     &Fact::valueChanged, this, &StructureScanComplexItem::_signalTopBottomAltChanged);
    connect(&_layersFact,           &Fact::valueChanged, this, &StructureScanComplexItem::_signalTopBottomAltChanged);

    connect(&_structureHeightFact,                  &Fact::valueChanged,    this, &StructureScanComplexItem::_recalcLayerInfo);
    connect(&_scanBottomAltFact,                    &Fact::valueChanged,    this, &StructureScanComplexItem::_recalcLayerInfo);
    connect(_cameraCalc.adjustedFootprintFrontal(), &Fact::valueChanged,    this, &StructureScanComplexItem::_recalcLayerInfo);

    connect(&_entranceAltFact, &Fact::valueChanged, this, &StructureScanComplexItem::_updateCoordinateAltitudes);

    connect(&_structurePolygon, &QGCMapPolygon::dirtyChanged,   this, &StructureScanComplexItem::_polygonDirtyChanged);
    connect(&_structurePolygon, &QGCMapPolygon::pathChanged,    this, &StructureScanComplexItem::_rebuildFlightPolygon);

    connect(&_structurePolygon, &QGCMapPolygon::countChanged,   this, &StructureScanComplexItem::_updateLastSequenceNumber);
    connect(&_layersFact,       &Fact::valueChanged,            this, &StructureScanComplexItem::_updateLastSequenceNumber);

    connect(&_flightPolygon,    &QGCMapPolygon::pathChanged,    this, &StructureScanComplexItem::_flightPathChanged);

    connect(_cameraCalc.distanceToSurface(),    &Fact::valueChanged,                this, &StructureScanComplexItem::_rebuildFlightPolygon);

    connect(&_flightPolygon,                        &QGCMapPolygon::pathChanged,    this, &StructureScanComplexItem::_recalcCameraShots);
    connect(_cameraCalc.adjustedFootprintSide(),    &Fact::valueChanged,            this, &StructureScanComplexItem::_recalcCameraShots);
    connect(&_layersFact,                           &Fact::valueChanged,            this, &StructureScanComplexItem::_recalcCameraShots);

    connect(&_cameraCalc, &CameraCalc::isManualCameraChanged, this, &StructureScanComplexItem::_updateGimbalPitch);

    _recalcLayerInfo();

    if (!kmlOrShpFile.isEmpty()) {
        _structurePolygon.loadKMLOrSHPFile(kmlOrShpFile);
        _structurePolygon.setDirty(false);
    }

    setDirty(false);
}

void StructureScanComplexItem::_setScanDistance(double scanDistance)
{
    if (!qFuzzyCompare(_scanDistance, scanDistance)) {
        _scanDistance = scanDistance;
        emit complexDistanceChanged();
    }
}

void StructureScanComplexItem::_setCameraShots(int cameraShots)
{
    if (_cameraShots != cameraShots) {
        _cameraShots = cameraShots;
        emit cameraShotsChanged(this->cameraShots());
    }
}

void StructureScanComplexItem::_clearInternal(void)
{
    setDirty(true);

    emit specifiesCoordinateChanged();
    emit lastSequenceNumberChanged(lastSequenceNumber());
}

void StructureScanComplexItem::_updateLastSequenceNumber(void)
{
    emit lastSequenceNumberChanged(lastSequenceNumber());
}

int StructureScanComplexItem::lastSequenceNumber(void) const
{
    // Each structure layer contains:
    //  1 waypoint for each polygon vertex + 1 to go back to first polygon vertex for each layer
    //  Two commands for camera trigger start/stop
    int layerItemCount = _flightPolygon.count() + 1 + 2;

    // Take into account the number of layers
    int multiLayerItemCount = layerItemCount * _layersFact.rawValue().toInt();

    // +2 for ROI_WPNEXT_OFFSET and ROI_NONE commands
    // +2 for entrance/exit waypoints
    int itemCount = multiLayerItemCount + 2 + 2;

    return _sequenceNumber + itemCount - 1;
}

void StructureScanComplexItem::setDirty(bool dirty)
{
    if (_dirty != dirty) {
        _dirty = dirty;
        emit dirtyChanged(_dirty);
    }
}

void StructureScanComplexItem::save(QJsonArray&  missionItems)
{
    QJsonObject saveObject;

    // Header
    saveObject[JsonHelper::jsonVersionKey] =                    3;
    saveObject[VisualMissionItem::jsonTypeKey] =                VisualMissionItem::jsonTypeComplexItemValue;
    saveObject[ComplexMissionItem::jsonComplexItemTypeKey] =    jsonComplexItemTypeValue;

    saveObject[_entranceAltName] =      _entranceAltFact.rawValue().toDouble();
    saveObject[scanBottomAltName] =     _scanBottomAltFact.rawValue().toDouble();
    saveObject[structureHeightName] =   _structureHeightFact.rawValue().toDouble();
    saveObject[layersName] =            _layersFact.rawValue().toDouble();
    saveObject[gimbalPitchName] =       _gimbalPitchFact.rawValue().toDouble();
    saveObject[startFromTopName] =      _startFromTopFact.rawValue().toBool();

    QJsonObject cameraCalcObject;
    _cameraCalc.save(cameraCalcObject);
    saveObject[_jsonCameraCalcKey] = cameraCalcObject;

    _structurePolygon.saveToJson(saveObject);

    missionItems.append(saveObject);
}

void StructureScanComplexItem::setSequenceNumber(int sequenceNumber)
{
    if (_sequenceNumber != sequenceNumber) {
        _sequenceNumber = sequenceNumber;
        emit sequenceNumberChanged(sequenceNumber);
        emit lastSequenceNumberChanged(lastSequenceNumber());
    }
}

bool StructureScanComplexItem::load(const QJsonObject& complexObject, int sequenceNumber, QString& errorString)
{
    QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { JsonHelper::jsonVersionKey,                   QJsonValue::Double, true },
        { VisualMissionItem::jsonTypeKey,               QJsonValue::String, true },
        { ComplexMissionItem::jsonComplexItemTypeKey,   QJsonValue::String, true },
        { QGCMapPolygon::jsonPolygonKey,                QJsonValue::Array,  true },
        { scanBottomAltName,                            QJsonValue::Double, true },
        { structureHeightName,                          QJsonValue::Double, true },
        { layersName,                                   QJsonValue::Double, true },
        { _jsonCameraCalcKey,                           QJsonValue::Object, true },
        { _entranceAltName,                             QJsonValue::Double, true },
        { gimbalPitchName,                              QJsonValue::Double, true },
        { startFromTopName,                             QJsonValue::Bool,   true },
    };
    if (!JsonHelper::validateKeys(complexObject, keyInfoList, errorString)) {
        return false;
    }

    _structurePolygon.clear();

    QString itemType = complexObject[VisualMissionItem::jsonTypeKey].toString();
    QString complexType = complexObject[ComplexMissionItem::jsonComplexItemTypeKey].toString();
    if (itemType != VisualMissionItem::jsonTypeComplexItemValue || complexType != jsonComplexItemTypeValue) {
        errorString = tr("%1 does not support loading this complex mission item type: %2:%3").arg(qgcApp()->applicationName()).arg(itemType).arg(complexType);
        return false;
    }

    int version = complexObject[JsonHelper::jsonVersionKey].toInt();
    if (version != 3) {
        errorString = tr("%1 version %2 not supported").arg(jsonComplexItemTypeValue).arg(version);
        return false;
    }

    setSequenceNumber(sequenceNumber);

    // Load CameraCalc first since it will trigger camera name change which will trounce gimbal angles
    if (!_cameraCalc.load(complexObject[_jsonCameraCalcKey].toObject(), errorString)) {
        return false;
    }

    _scanBottomAltFact.setRawValue      (complexObject[scanBottomAltName].toDouble());
    _layersFact.setRawValue             (complexObject[layersName].toDouble());
    _structureHeightFact.setRawValue    (complexObject[structureHeightName].toDouble());
    _startFromTopFact.setRawValue       (complexObject[startFromTopName].toBool());
    _entranceAltFact.setRawValue        (complexObject[startFromTopName].toDouble());
    _gimbalPitchFact.setRawValue        (complexObject[gimbalPitchName].toDouble());

    if (!_structurePolygon.loadFromJson(complexObject, true /* required */, errorString)) {
        _structurePolygon.clear();
        return false;
    }

    return true;
}

void StructureScanComplexItem::_flightPathChanged(void)
{
    // Calc bounding cube
    double north = 0.0;
    double south = 180.0;
    double east  = 0.0;
    double west  = 360.0;
    double bottom = 100000.;
    double top = 0.;
    QList<QGeoCoordinate> vertices = _flightPolygon.coordinateList();
    for (int i = 0; i < vertices.count(); i++) {
        QGeoCoordinate vertex = vertices[i];
        double lat = vertex.latitude()  + 90.0;
        double lon = vertex.longitude() + 180.0;
        north   = fmax(north, lat);
        south   = fmin(south, lat);
        east    = fmax(east,  lon);
        west    = fmin(west,  lon);
        bottom  = fmin(bottom, vertex.altitude());
        top     = fmax(top, vertex.altitude());
    }
    //-- Update bounding cube for airspace management control
    _setBoundingCube(QGCGeoBoundingCube(
        QGeoCoordinate(north - 90.0, west - 180.0, bottom),
        QGeoCoordinate(south - 90.0, east - 180.0, top)));

    emit coordinateChanged(coordinate());
    emit exitCoordinateChanged(exitCoordinate());
    emit greatestDistanceToChanged();
}

double StructureScanComplexItem::greatestDistanceTo(const QGeoCoordinate &other) const
{
    double greatestDistance = 0.0;
    QList<QGeoCoordinate> vertices = _flightPolygon.coordinateList();

    for (int i=0; i<vertices.count(); i++) {
        QGeoCoordinate vertex = vertices[i];
        double distance = vertex.distanceTo(other);
        if (distance > greatestDistance) {
            greatestDistance = distance;
        }
    }

    return greatestDistance;
}

bool StructureScanComplexItem::specifiesCoordinate(void) const
{
    return _flightPolygon.count() > 2;
}

void StructureScanComplexItem::appendMissionItems(QList<MissionItem*>& items, QObject* missionItemParent)
{
    int     seqNum =        _sequenceNumber;
    bool    startFromTop =  _startFromTopFact.rawValue().toBool();
    double  startAltitude = _scanBottomAltFact.rawValue().toDouble() + (startFromTop ? _structureHeightFact.rawValue().toDouble() : 0);

    MissionItem* item = nullptr;

    // Entrance waypoint
    QGeoCoordinate entranceExitCoord = _flightPolygon.vertexCoordinate(_entryVertex);
    item = new MissionItem(seqNum++,
                           MAV_CMD_NAV_WAYPOINT,
                           MAV_FRAME_GLOBAL_RELATIVE_ALT,
                           0,                                          // No hold time
                           0.0,                                        // No acceptance radius specified
                           0.0,                                        // Pass through waypoint
                           std::numeric_limits<double>::quiet_NaN(),   // Yaw unchanged
                           entranceExitCoord.latitude(),
                           entranceExitCoord.longitude(),
                           _entranceAltFact.rawValue().toDouble(),
                           true,                                       // autoContinue
                           false,                                      // isCurrentItem
                           missionItemParent);
    items.append(item);

    // Point camera at structure
    item = new MissionItem(seqNum++,
                           MAV_CMD_DO_SET_ROI_WPNEXT_OFFSET,
                           MAV_FRAME_MISSION,
                           0, 0, 0, 0,                             // param 1-4 not used
                           _gimbalPitchFact.rawValue().toDouble(),
                           0,                                      // Roll stays in standard orientation
                           90,                                     // 90 degreee yaw offset to point to structure
                           true,                                   // autoContinue
                           false,                                  // isCurrentItem
                           missionItemParent);
    items.append(item);

    // Set up for the first layer
    double  layerAltitude = startAltitude;
    double  halfLayerHeight = _cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble() / 2.0;
    if (startFromTop) {
        layerAltitude -= halfLayerHeight;
    } else {
        layerAltitude += halfLayerHeight;
    }

    for (int layer=0; layer<_layersFact.rawValue().toInt(); layer++) {
        bool addTriggerStart = true;

        bool done = false;
        int currentVertex = _entryVertex;
        int processedVertices = 0;
        do {
            QGeoCoordinate vertexCoord = _flightPolygon.vertexCoordinate(currentVertex);

            item = new MissionItem(seqNum++,
                                   MAV_CMD_NAV_WAYPOINT,
                                   MAV_FRAME_GLOBAL_RELATIVE_ALT,
                                   0,                                          // No hold time
                                   0.0,                                        // No acceptance radius specified
                                   0.0,                                        // Pass through waypoint
                                   std::numeric_limits<double>::quiet_NaN(),   // Yaw unchanged
                                   vertexCoord.latitude(),
                                   vertexCoord.longitude(),
                                   layerAltitude,
                                   true,                                       // autoContinue
                                   false,                                      // isCurrentItem
                                   missionItemParent);
            items.append(item);

            // Start camera triggering after first waypoint in layer
            if (addTriggerStart) {
                addTriggerStart = false;
                item = new MissionItem(seqNum++,
                                       MAV_CMD_DO_SET_CAM_TRIGG_DIST,
                                       MAV_FRAME_MISSION,
                                       _cameraCalc.adjustedFootprintSide()->rawValue().toDouble(),  // trigger distance
                                       0,                                                           // shutter integration (ignore)
                                       1,                                                           // trigger immediately when starting
                                       0, 0, 0, 0,                                                  // param 4-7 unused
                                       true,                                                        // autoContinue
                                       false,                                                       // isCurrentItem
                                       missionItemParent);
                items.append(item);
            }

            // Move to next vertext
            currentVertex++;
            if (currentVertex >= _flightPolygon.count()) {
                currentVertex = 0;
            }

            // Have we processed all the vertices
            processedVertices++;
            done = processedVertices == _flightPolygon.count() + 1;
        } while (!done);

        // Stop camera triggering after last waypoint in layer
        item = new MissionItem(seqNum++,
                               MAV_CMD_DO_SET_CAM_TRIGG_DIST,
                               MAV_FRAME_MISSION,
                               0,           // stop triggering
                               0,           // shutter integration (ignore)
                               0,           // trigger immediately when starting
                               0, 0, 0, 0,  // param 4-7 unused
                               true,        // autoContinue
                               false,       // isCurrentItem
                               missionItemParent);
        items.append(item);

        // Move to next layer altitude
        if (startFromTop) {
            layerAltitude -= halfLayerHeight * 2;
        } else {
            layerAltitude += halfLayerHeight * 2;
        }
    }

    // Return camera to neutral position
    item = new MissionItem(seqNum++,
                           MAV_CMD_DO_SET_ROI_NONE,
                           MAV_FRAME_MISSION,
                           0, 0, 0,0, 0, 0, 0,                 // param 1-7 not used
                           true,                               // autoContinue
                           false,                              // isCurrentItem
                           missionItemParent);
    items.append(item);

    // Exit waypoint
    item = new MissionItem(seqNum++,
                           MAV_CMD_NAV_WAYPOINT,
                           MAV_FRAME_GLOBAL_RELATIVE_ALT,
                           0,                                          // No hold time
                           0.0,                                        // No acceptance radius specified
                           0.0,                                        // Pass through waypoint
                           std::numeric_limits<double>::quiet_NaN(),   // Yaw unchanged
                           entranceExitCoord.latitude(),
                           entranceExitCoord.longitude(),
                           _entranceAltFact.rawValue().toDouble(),
                           true,                                       // autoContinue
                           false,                                      // isCurrentItem
                           missionItemParent);
    items.append(item);

}

int StructureScanComplexItem::cameraShots(void) const
{
    return _cameraShots;
}

void StructureScanComplexItem::setMissionFlightStatus(MissionController::MissionFlightStatus_t& missionFlightStatus)
{
    ComplexMissionItem::setMissionFlightStatus(missionFlightStatus);
    if (!qFuzzyCompare(_cruiseSpeed, missionFlightStatus.vehicleSpeed)) {
        _cruiseSpeed = missionFlightStatus.vehicleSpeed;
        emit timeBetweenShotsChanged();
    }
}

void StructureScanComplexItem::_setDirty(void)
{
    setDirty(true);
}

void StructureScanComplexItem::applyNewAltitude(double newAltitude)
{
    _entranceAltFact.setRawValue(newAltitude);
}

void StructureScanComplexItem::_polygonDirtyChanged(bool dirty)
{
    if (dirty) {
        setDirty(true);
    }
}

double StructureScanComplexItem::timeBetweenShots(void)
{
    return _cruiseSpeed == 0 ? 0 : _cameraCalc.adjustedFootprintSide()->rawValue().toDouble() / _cruiseSpeed;
}

QGeoCoordinate StructureScanComplexItem::coordinate(void) const
{
    if (_flightPolygon.count() > 0) {
        QGeoCoordinate coord = _flightPolygon.vertexCoordinate(_entryVertex);
        coord.setAltitude(_entranceAltFact.rawValue().toDouble());
        return coord;
    } else {
        return QGeoCoordinate();
    }
}

QGeoCoordinate StructureScanComplexItem::exitCoordinate(void) const
{
    return coordinate();
}

void StructureScanComplexItem::_updateCoordinateAltitudes(void)
{
    emit coordinateChanged(coordinate());
    emit exitCoordinateChanged(exitCoordinate());
}

void StructureScanComplexItem::rotateEntryPoint(void)
{
    _entryVertex++;
    if (_entryVertex >= _flightPolygon.count()) {
        _entryVertex = 0;
    }
    emit coordinateChanged(coordinate());
    emit exitCoordinateChanged(exitCoordinate());
}

void StructureScanComplexItem::_rebuildFlightPolygon(void)
{
    // While this is happening all hell breaks loose signal-wise which can cause a bad vertex reference.
    // So we reset to a safe value first and then double check validity when putting it back
    int savedEntryVertex = _entryVertex;
    _entryVertex = 0;

    _flightPolygon = _structurePolygon;
    _flightPolygon.offset(_cameraCalc.distanceToSurface()->rawValue().toDouble());

    if (savedEntryVertex >= _flightPolygon.count()) {
        _entryVertex = 0;
    } else {
        _entryVertex = savedEntryVertex;
    }
    emit coordinateChanged(coordinate());
    emit exitCoordinateChanged(exitCoordinate());
}

void StructureScanComplexItem::_recalcCameraShots(void)
{
    double triggerDistance = _cameraCalc.adjustedFootprintSide()->rawValue().toDouble();
    if (triggerDistance == 0) {
        _setCameraShots(0);
        return;
    }

    if (_flightPolygon.count() < 3) {
        _setCameraShots(0);
        return;
    }

    // Determine the distance for each polygon traverse
    double distance = 0;
    for (int i=0; i<_flightPolygon.count(); i++) {
        QGeoCoordinate coord1 = _flightPolygon.vertexCoordinate(i);
        QGeoCoordinate coord2 = _flightPolygon.vertexCoordinate(i + 1 == _flightPolygon.count() ? 0 : i + 1);
        distance += coord1.distanceTo(coord2);
    }
    if (distance == 0.0) {
        _setCameraShots(0);
        return;
    }

    int cameraShots = static_cast<int>(distance / triggerDistance);
    _setCameraShots(cameraShots * _layersFact.rawValue().toInt());
}

void StructureScanComplexItem::_recalcLayerInfo(void)
{
    double surfaceHeight = qMax(_structureHeightFact.rawValue().toDouble() - _scanBottomAltFact.rawValue().toDouble(), 0.0);

    // Layer count is calculated from surface and layer heights
    _layersFact.setRawValue(qMax(qCeil(surfaceHeight / _cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble()), 1));
}

void StructureScanComplexItem::_updateGimbalPitch(void)
{
    if (!_cameraCalc.isManualCamera()) {
        _gimbalPitchFact.setRawValue(0);
    }
}

double StructureScanComplexItem::bottomFlightAlt(void)
{
    if (_startFromTopFact.rawValue().toBool()) {
        // Structure Height minus the topmost layers
        double  layerIncrement = (_cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble() / 2.0) + ((_layersFact.rawValue().toInt() - 1) * _cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble());
        return _structureHeightFact.rawValue().toDouble() - layerIncrement;
    } else {
        // Bottom alt plus half the height of a layer
        double  layerIncrement = _cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble() / 2.0;
        return _scanBottomAltFact.rawValue().toDouble() + layerIncrement;
    }
}

double StructureScanComplexItem:: topFlightAlt(void)
{
    if (_startFromTopFact.rawValue().toBool()) {
        // Structure Height minus half the layer height
        double  layerIncrement = _cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble() / 2.0;
        return _structureHeightFact.rawValue().toDouble() - layerIncrement;
    } else {
        // Bottom alt plus all layers
        double  layerIncrement = (_cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble() / 2.0) + ((_layersFact.rawValue().toInt() - 1) * _cameraCalc.adjustedFootprintFrontal()->rawValue().toDouble());
        return _scanBottomAltFact.rawValue().toDouble() + layerIncrement;
    }
}

void StructureScanComplexItem::_signalTopBottomAltChanged(void)
{
    emit topFlightAltChanged();
    emit bottomFlightAltChanged();
}