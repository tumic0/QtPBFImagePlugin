# QtPBFImagePlugin
Qt image plugin for displaying Mapbox vector tiles

## Description
QtPBFImagePlugin is a Qt image plugin that enables applications capable of
displaying raster MBTiles maps to also display PBF vector tiles without
(almost, see usage) any application modifications.

Standard Mapbox GL Styles are used for styling the maps. Most style features
used by [Maputnik](http://editor.openmaptiles.org) are supported. The style
is loaded from /usr/share/pbf/style.json or ~/.pbf/style.json if it exists.

## Usage
Due to a major design flaw in the Mapbox vector tiles specification - the zoom
is not part of the PBF data - the plugin can not be used "as is", but passing
the zoom level is necessary. This is done by exploiting the optional format
parameter of the QImage constructor or the QImage::fromData() or
QPixmap::loadFromData() functions. The zoom number is passed as ASCII string
to the functions:
```cpp
QPixmap pm;
pm.loadFromData(tileData, QString::number(zoom).toLatin1());
```

## Build
Build requirements:
* QT 5.x
* Google Protocol Buffers (protobuf-lite)

Build steps:
```shell
qmake pbfplugin.pro
make
```

## Install
Copy the plugin to the system Qt image plugins path to make it work. You may
also set the QT_PLUGIN_PATH system variable before starting the application.

## Limitations
As the plugin only has isolated info about a single tile image, texts
overlapping to neighbour tiles can not be displayed or their overlapping can
not be avoided (the plugin uses the first approach). This is a principal
constraint that can't be evaded.

## Status
A picture is worth a thousand words. Data and styles from https://openmaptiles.org.
#### OSM-liberty
![osm-liberty 2](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-2.png)
![osm-liberty 5](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-5.png)
![osm-liberty 9](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-9.png)
![osm-liberty 11](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-11.png)
![osm-liberty 14](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-14.png)

#### Klokantech-basic
![klokantech-basic 2](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-2.png)
![klokantech-basic 4](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-4.png)
![klokantech-basic 9](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-9.png)
![klokantech-basic 13](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-13.png)
![klokantech-basic 14](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-14.png)

