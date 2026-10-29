# Marca Drive, um produto DBPixel (manual 9.2). Lido pelo THEME.cmake no lugar do OWNCLOUD.cmake quando a pasta branding/ existe.
set( APPLICATION_NAME       "Drive" )
set( APPLICATION_SHORTNAME  "DBPixelDrive" )
set( APPLICATION_EXECUTABLE "dbpixeldrive" )
set( APPLICATION_DOMAIN     "dbpixel.com.br" )
set( APPLICATION_VENDOR     "DBPixel" )
set( APPLICATION_ICON_NAME  "drive" )
set( APPLICATION_VIRTUALFILE_SUFFIX "dbpixel" CACHE STRING "Virtual file suffix (not including the .)")

set( LINUX_PACKAGE_SHORTNAME "dbpixeldrive" )

set( THEME_CLASS            "DBPixelTheme" )
set( THEME_INCLUDE          "${OEM_THEME_DIR}/dbpixeltheme.h" )
set( APPLICATION_REV_DOMAIN "br.com.dbpixel.drive" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

# sem atualizador automático: a DBPixel ainda não tem servidor de atualizações
set( WITHOUT_AUTO_UPDATER ON )
option( WITH_CRASHREPORTER "Build crashreporter" OFF )
