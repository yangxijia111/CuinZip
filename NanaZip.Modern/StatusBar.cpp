#include "pch.h"
#include "StatusBar.h"
#include "StatusBar.g.cpp"

using namespace winrt::NanaZip::Modern::implementation;

DEPENDENCY_PROPERTY_SOURCE_BOX(
    TextPrimary,
    winrt::hstring,
    StatusBar,
    winrt::NanaZip::Modern::StatusBar
);

DEPENDENCY_PROPERTY_SOURCE_BOX(
    TextArchive,
    winrt::hstring,
    StatusBar,
    winrt::NanaZip::Modern::StatusBar
);
