#pragma once

#include "SponsorPage.g.h"

#include <Windows.h>

namespace winrt
{
    using Windows::Foundation::IInspectable;
    using Windows::UI::Xaml::RoutedEventArgs;
}

namespace winrt::NanaZip::Modern::implementation
{
    struct SponsorPage : SponsorPageT<SponsorPage>
    {
    public:

        SponsorPage(
            _In_opt_ HWND WindowHandle = nullptr);

        void InitializeComponent();

        void CuinZipGitHubButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void NanaZipGitHubButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void SevenZipWebsiteButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

    private:

        HWND m_WindowHandle;
    };
}
