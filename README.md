# QtPBFImagePlugin
Qt image plugin for displaying Mapbox vector tiles

## Description
QtPBFImagePlugin is a Qt image plugin that enables applications capable of
displaying raster MBTiles maps or raster XYZ online maps to also display
PBF(MVT) vector tiles without (almost, see usage) any application modifications.

Standard Mapbox GL Styles are used for styling the maps. Most relevant style
features used by [Maputnik](https://maputnik.github.io/editor) are supported.
A default fallback style (OSM-Liberty) for OpenMapTiles is part of the plugin.

"Plain" PBF files as well as gzip compressed files (as used in MBTiles) are
supported by the plugin. The tile size is (since version 2.0 of the plugin) 512px
to fit the styles and available data (OpenMapTiles, Mapbox tiles).

## Usage
Due to a major design flaw in the Mapbox vector tiles specification - the zoom
is not part of the PBF data - the plugin can not be used "as is", but passing
the zoom level is necessary. This is done by exploiting the optional *format*
parameter of the QImage constructor or the QImage::fromData() or
QPixmap::loadFromData() functions. The zoom number is passed as ASCII string
to the functions:
```cpp
QPixmap pm;
pm.loadFromData(tileData, QByteArray::number(zoom));
```
The plugin supports vector scaling using QImageReader's setScaledSize() method,
so when used like in the following example:
```cpp
QImageReader reader(file, QByteArray::number(zoom));
reader.setScaledSize(QSize(1024, 1024));
reader.read(&image);
```
you will get 1024x1024px tiles with a pixel ratio of 2 (= HiDPI tiles).

Since version 2.7 tile "overzoom" is supported. If you set *format* to
"$zoom;$overzoom":
```cpp
QPixmap pm;
pm.loadFromData(tileData, QByteArray::number(zoom) + ';' +
                          QByteArray::number(overzoom));
```
you will get (512<<overzoom)x(512<<overzoom)px tiles with a pixel ratio of 1.
When overzoom is combined with setScaledSize(), the base size is the overzoomed
tile size.

For a sample code see the [pbf2png](https://github.com/tumic0/pbf2png)
conversion utility.

## Styles
The map style is loaded from the
[$AppDataLocation](http://doc.qt.io/qt-5/qstandardpaths.html)/style/style.json
file on plugin load. If the style uses a sprite, the sprite JSON file must
be named `sprite.json` and the sprite image `sprite.png` and both files must be
placed in the same directory as the style itself. *A style compatible with the
tiles data schema (Mapbox, OpenMapTiles, Tilezen, Ordnance Survey, Esri, ...)
must be used.*

For a list of "ready to use" styles see the
[QtPBFImagePlugin-styles](https://github.com/tumic0/QtPBFImagePlugin-styles)
repository.

## Build
### Requirements
* Qt5 >= 5.11 or Qt6
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
also set the QT_PLUGIN_PATH system variable before starting the application. For
Linux, there are RPM and DEB [packages](https://build.opensuse.org/project/show/home:tumic:QtPBFImagePlugin)
for most common distros available on OBS.

## Limitations
* Only data that is part of the PBF file is displayed. External layers defined in the
style are ignored.
* Text PBF features must have a unique id (OpenMapTiles >= v3.7) for the text layout
algorithm to work properly.
* Expressions are not supported in the styles, only property functions are implemented.

## Changelog
[Changelog](https://build.opensuse.org/package/view_file/home:tumic:QtPBFImagePlugin/QtPBFImagePlugin/libqt5-qtpbfimageformat.changes)

## Status
A picture is worth a thousand words.
#### OpenMapTiles

* Data: [MapTiler](https://github.com/tumic0/GPXSee-maps/blob/master/World/MapTiler.tpl)
* Style: [OSM-liberty](https://github.com/tumic0/QtPBFImagePlugin-styles/blob/master/OpenMapTiles/osm-liberty/style.json)

![osm-liberty 5](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-5.png)
![osm-liberty 8](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-8.png)
![osm-liberty 12](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-12.png)
![osm-liberty 14](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-14.png)
![osm-liberty 15](https://tumic0.github.io/QtPBFImagePlugin/images/osm-liberty-15.png)

#### Mapbox

* Data: [Mapbox](https://github.com/tumic0/GPXSee-maps/blob/master/World/Mapbox.tpl)
* Style: [Bright](https://github.com/tumic0/QtPBFImagePlugin-styles/blob/master/Mapbox/bright/style.json)

![bright 4](https://tumic0.github.io/QtPBFImagePlugin/images/bright-4.png)
![bright 6](https://tumic0.github.io/QtPBFImagePlugin/images/bright-6.png)
![bright 13](https://tumic0.github.io/QtPBFImagePlugin/images/bright-13.png)
![bright 15](https://tumic0.github.io/QtPBFImagePlugin/images/bright-15.png)
![bright 17](https://tumic0.github.io/QtPBFImagePlugin/images/bright-17.png)

#### Tilezen

* Data: [HERE](https://github.com/tumic0/GPXSee-maps/blob/master/World/here-vector.tpl)
* Style: [Apollo-Bright](https://github.com/tumic0/QtPBFImagePlugin-styles/blob/master/Tilezen/apollo-bright/style.json)

![apollo-bright 4](https://tumic0.github.io/QtPBFImagePlugin/images/apollo-bright-4.png)
![apollo-bright 6](https://tumic0.github.io/QtPBFImagePlugin/images/apollo-bright-6.png)
![apollo-bright 12](https://tumic0.github.io/QtPBFImagePlugin/images/apollo-bright-12.png)
![apollo-bright 15](https://tumic0.github.io/QtPBFImagePlugin/images/apollo-bright-15.png)
![apollo-bright 16](https://tumic0.github.io/QtPBFImagePlugin/images/apollo-bright-16.png)

## Applications using QtPBFImagePlugin
* [GPXSee](https://www.gpxsee.org)
