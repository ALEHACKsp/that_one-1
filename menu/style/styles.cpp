#include "styles.h"
#include "../../tools/draw/cdrawmanager.h"

int DefaultStyle::ControlPanel( int x, int y, int w, int h, const char* name ) {
  DrawManager::OutlineRect( x, y, w, h, Color( 50, 50, 55 ) );
  int textW = 0, textH = 15;

  if( name ) {
    Int::Surface->GetTextSize( gFonts.calibri_light_small, ToWstring( name ).c_str(), textW, textH );
  }

  DrawManager::DrawRect( x + 1, y + 1, w - 2, textH, Color( 25 ) );

  if( name ) {
    DrawManager::DrawString( x + ( ( w / 2 ) - ( textW / 2 ) ), y, Color( 150 ), name, gFonts.calibri_light_small );
  }

  DrawManager::DrawLine( x, y + textH, x + w, y + textH, Color( 50, 50, 55 ) );
  x += 1, y += textH + 1, w -= 2, h -= textH + 2;
  DrawManager::DrawRect( x, y, w, h, Color( 30, 30, 33 ) );
  return textH + 1;
}

int DefaultStyle::DialogButton( int x, int y, int w, const char* text, bool mouseOver ) {
  DrawManager::DrawRect( x, y, w, 15, Color( 42, 42, 48 ) );
  DrawManager::OutlineRect( x, y, w, 15, Color( 60 ) );
  DrawManager::DrawString( x + 3, y, mouseOver ? Color( 90, 150, 225 ) : Color( 125, 125, 130 ), text, gFonts.verdana_bold );
  return 15;
}

void DefaultStyle::Dialog( int x, int y, int w, int h ) {
  DrawManager::OutlineRect( x, y, w, h, Color( 58, 58, 70 ) );
  DrawManager::DrawRect( x + 1, y + 1, w - 2, h - 2, Color( 36, 36, 40 ) );
}

constexpr int TOPBAR = 30;
int DefaultStyle::TopBar( int x, int y, int w, const char* title ) {
  // Dark topbar
  DrawManager::DrawRect( x, y, w, TOPBAR, Color( 20 ) );
  // Some small shading below the topbar
  DrawManager::DrawLine( x, y + TOPBAR - 1, x + w, y + TOPBAR - 1, Color( 30 ) );
  DrawManager::DrawLine( x, y + TOPBAR - 2, x + w, y + TOPBAR - 2, Color( 0 ) );

  if( title ) {
    DrawManager::DrawString( x + 20, y + 2, Color( 120 ), title, gFonts.calibri );
  }

  return TOPBAR;
}

