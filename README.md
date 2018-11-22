# QtPBFImagePlugin
Qt image plugin for displaying Mapbox vector tiles

## Description
QtPBFImagePlugin is a Qt image plugin that enables applications capable of
displaying raster MBTiles maps or raster XYZ online maps to also display PBF
vector tiles without (almost, see usage) any application modifications.

Standard Mapbox GL Styles are used for styling the maps. Most relevant style
features used by [Maputnik](http://editor.openmaptiles.org) are supported.
The style is loaded from the
[$AppDataLocation](http://doc.qt.io/qt-5/qstandardpaths.html)/style/style.json
file on plugin load. If the style uses a sprite, the sprite JSON file must
be named sprite.json and the sprite image sprite.png and both files must be
placed in the same directory as the style itself. A default fallback style
(OSM-Liberty) for OpenMapTiles is part of the plugin.

## Usage
Due to a major design flaw in the Mapbox vector tiles specification - the zoom
is not part of the PBF data - the plugin can not be used "as is", but passing
the zoom level is necessary. This is done by exploiting the optional *format*
parameter of the QImage constructor or the QImage::fromData() or
QPixmap::loadFromData() functions. The zoom number is passed as ASCII string
to the functions:
```cpp
QPixmap pm;
pm.loadFromData(tileData, QString::number(zoom).toLatin1());
```
The plugin supports vector scaling using QImageReader's setScaledSize() method,
so when used like in the following example:
```cpp
QImageReader reader(file, QString::number(zoom).toLatin1());
reader.setScaledSize(QSize(512, 512));
reader.read(&image);
```
you will get 512x512px tiles with a pixel ratio of 2 (= HiDPI tiles).

## Build
### Requirements
* Qt 5
* Google Protocol Buffers (protobuf-lite)
* Zlib

### Build steps
#### Linux
```shell
qmake pbfplugin.pro
make
```
#### Windows
```shell
qmake PROTOBUF=path/to/protobuf ZLIB=path/to/zlib pbfplugin.pro
nmake
```
#### OS X
```shell
qmake PROTOBUF=path/to/protobuf pbfplugin.pro
make
```

## Install
Copy the plugin to the system Qt image plugins path to make it work. You may
also set the QT_PLUGIN_PATH system variable before starting the application.

## Limitations
Text PBF features must have a unique id (OpenMapTiles >= v3.7) for the text layout
algorithm to work properly.

## Status
A picture is worth a thousand words. Data and styles from https://openmaptiles.org.
#### OSM-liberty
![osm-liberty 2](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-2.png)
![osm-liberty 5](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-5.png)
![osm-liberty 8](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-8.png)
![osm-liberty 11](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-11.png)
![osm-liberty 14](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-14.png)

#### Klokantech-basic
![klokantech-basic 2](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-2.png)
![klokantech-basic 4](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-4.png)
![klokantech-basic 8](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-8.png)
![klokantech-basic 13](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-13.png)
![klokantech-basic 14](https://tumic0.github.io/QtPBFImagePlugin/images/klokantech-basic-14.png)

