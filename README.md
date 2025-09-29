# QtPBFImagePlugin
Qt image plugin for displaying Mapbox vector tiles

## Description
QtPBFImagePlugin is a Qt image plugin that enables applications capable of
displaying raster MBTiles maps or raster XYZ online maps to also display
PBF(MVT) vector tiles without (almost, see usage) any application modifications.

Standard Mapbox GL Styles are used for styling the maps. Most relevant style
features used by [Maputnik](https://maputnik.github.io/editor) are supported.
A set of default styles for the OpenMapTiles, Mapbox, Protomaps and Versatiles
tile shemes is part of the plugin.

The default tile size is 512px.

## Usage
Due to a major design flaw in the Mapbox vector tiles specification - the zoom
is not part of the PBF data - the plugin can not be used "as is", but passing
the zoom level is necessary. This is done by exploiting the optional *format*
parameter of the QImage constructor or the *QImage::loadFromData()* or
*QPixmap::loadFromData()* functions. The zoom number is passed as ASCII string
to the functions:
```cpp
QImage img;
img.loadFromData(data, QByteArray::number(zoom));
```

For a complete code sample see the [pbf2png](https://github.com/tumic0/pbf2png)
conversion utility.

### HiDPI
The plugin supports vector scaling using the *QImageReader::setScaledSize()*
method, so when used like in the following example:
```cpp
QImage img;
QImageReader reader(file, QByteArray::number(zoom));
reader.setScaledSize(QSize(1024, 1024));
reader.read(&img);
```
you will get 1024x1024px tiles with a pixel ratio of 2 (= HiDPI tiles). The maximal
tile size is 4096x4096.

### Overzoom
Since version 3 of the plugin tile overzoom is supported. If you set *format*
to `$zoom;$overzoom`:
```cpp
QImage img;
QByteArray fmt(QByteArray::number(zoom) + ';' + QByteArray::number(overzoom));
img.loadFromData(data, fmt);
```
you will get (512<<overzoom)x(512<<overzoom)px tiles with a pixel ratio of 1.
When overzoom is combined with *setScaledSize()*, the base size is the overzoomed
tile size.

### Style selection
Since version 5 of the plugin style selection is supported. If you set *format*
to `$zoom;$overzoom;$style`:
```cpp
QImage img;
QByteArray fmt(QByteArray::number(zoom) + ';' + QByteArray::number(overzoom) \
  + ';' + QByteArray::number(style));
img.loadFromData(data, fmt);
```
the style-th style available will be used to render the tile.

To retrieve a list of available styles, call *QImageReader::text()* with the
"Description" key:
```cpp
QImageReader reader(&imageData);
QString info(reader.text("Description"));
```
This will fill *info* with a JSON array like:
```json
[
  {
    "layers": [
      "aerodrome_label",
      "aeroway",
      "boundary",
      "building",
      "landcover",
      "landuse",
      "park",
      "place",
      "poi",
      "transportation",
      "transportation_name",
      "water",
      "water_name",
      "waterway"
    ],
    "name": "OpenMapTiles"
  },
  {
    "layers": [
      "boundaries",
      "buildings",
      "earth",
      "landcover",
      "landuse",
      "places",
      "pois",
      "roads",
      "water"
    ],
    "name": "Protomaps"
  }
]
```

## Styles
The map styles are loaded from subdirectories of the
[$AppDataLocation](http://doc.qt.io/qt-5/qstandardpaths.html)/style
directory on plugin load, one style per subdirectory. If the style uses a sprite,
the sprite JSON file must be named `sprite.json` and the sprite image `sprite.png`
and both files must be placed in the same directory as the style itself.

For a set of compatible styles for various different tile schemes see the
[QtPBFImagePlugin-styles](https://github.com/tumic0/QtPBFImagePlugin-styles)
repository.

## Build
### Prerequisites
* Qt5 >= 5.15 or Qt6 (Android builds require Qt6)

### Steps
#### Linux, OS X and Android
```shell
qmake pbfplugin.pro
make
```
#### Windows
```shell
qmake pbfplugin.pro
nmake
```

## Install
Copy the plugin to the system Qt image plugins path to make it work. You may
also set the QT_PLUGIN_PATH system variable before starting the application.
For Linux, there are RPM and DEB [packages](https://build.opensuse.org/project/show/home:tumic:QtPBFImagePlugin)
for most common distros available on OBS.

## Limitations
* Only data that is part of the PBF file is displayed. External layers defined
in the style are ignored.
* Text PBF features must have a unique id (OpenMapTiles >= v3.7) for the text
layout algorithm to work properly. Additionally, the tile buffer must be large
enough to contain all neighboring text features overlapping to the tile bounds
(only data from the tile itself can be drawn to the resulting image).
* Expressions are not supported in the styles, only the original GL zoom
functions are implemented.

## Changelog
[Changelog](https://build.opensuse.org/projects/home:tumic:QtPBFImagePlugin/packages/QtPBFImagePlugin/files/qt6-qtpbfimageformat.changes)

## Status
A picture is worth a thousand words.
#### OpenMapTiles

* Data: [MapTiler](https://github.com/tumic0/GPXSee-maps/blob/master/World/MapTiler-OpenMapTiles.tpl)
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
