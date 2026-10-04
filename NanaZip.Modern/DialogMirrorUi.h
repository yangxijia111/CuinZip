#pragma once

// CuinZip P1-3:对话框镜像引擎的 Modern 侧辅助。
// 封装 K7_DIALOG_MIRROR_ENGINE 的读取(缓冲区管理、快照结构)
// 与标签加速键清理,供 CompressDialogPage/ExtractDialogPage 共用。

#include "pch.h"

#include "NanaZip.Modern.h"
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <string>
#include <vector>

namespace winrt::NanaZip::Modern::implementation::MirrorUi
{
    // Editable paths need the Windows Unicode path limit, independently of
    // the fixed-size display slots used for each history-list entry.
    constexpr UINT MaxControlText = 32768;

    // ComboBox.Text may still contain the committed value while its focused
    // editor holds a new value. Read the live TextBox, including intentional
    // deletion, before rebuilding the control or submitting the dialog.
    inline winrt::Windows::UI::Xaml::Controls::TextBox FindComboEditor(
        winrt::Windows::UI::Xaml::DependencyObject const& root)
    {
        using winrt::Windows::UI::Xaml::Controls::TextBox;
        using winrt::Windows::UI::Xaml::Media::VisualTreeHelper;
        if (auto editor = root.try_as<TextBox>())
            return editor;
        const int count = VisualTreeHelper::GetChildrenCount(root);
        for (int i = 0; i < count; ++i)
        {
            if (auto editor = FindComboEditor(VisualTreeHelper::GetChild(root, i)))
                return editor;
        }
        return nullptr;
    }

    inline std::wstring ReadEditableComboText(
        winrt::Windows::UI::Xaml::Controls::ComboBox const& combo)
    {
        if (auto editor = FindComboEditor(combo))
            return std::wstring(editor.Text());
        return std::wstring(combo.Text());
    }

    // 去除 "&" 加速键与 "(&X)" 形式的括号加速键,供 Fluent 标签使用
    inline winrt::hstring StripAccelerator(std::wstring_view source)
    {
        std::wstring result(source);
        for (std::size_t position;
            (position = result.find(L"(&")) != std::wstring::npos;)
        {
            std::size_t end = result.find(L')', position);
            if (end == std::wstring::npos)
                break;
            result.erase(position, end - position + 1);
        }
        result.erase(std::remove(result.begin(), result.end(), L'&'),
            result.end());
        return winrt::hstring(result);
    }

    // 在 StripAccelerator 基础上再去掉结尾的 ':'
    inline winrt::hstring StripAcceleratorAndColon(std::wstring_view source)
    {
        std::wstring raw(StripAccelerator(source));
        while (!raw.empty() && (raw.back() == L':' || raw.back() == L' '))
            raw.pop_back();
        return winrt::hstring(raw);
    }

    struct ComboSnapshot
    {
        std::vector<winrt::hstring> Items;
        std::vector<LPARAM> ItemData;
        int Selection = -1;
        std::wstring Text;
        bool Enabled = true;
        bool Visible = true;
        bool Ok = false;
    };

    inline ComboSnapshot ReadCombo(
        const K7_DIALOG_MIRROR_ENGINE* engine,
        UINT controlId)
    {
        ComboSnapshot snapshot;
        if (!engine || !engine->ReadCombo || !engine->Context)
            return snapshot;

        UINT capacity = engine->ReadCombo(engine->Context, controlId,
            nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr, nullptr);
        if (capacity == (UINT)-1)
            return snapshot;
        std::vector<wchar_t> buffer(
            (size_t)capacity * K7_DIALOG_MIRROR_ITEM_TEXT);
        std::vector<LPARAM> itemData(capacity);
        std::vector<wchar_t> text(MaxControlText, L'\0');
        int selection = -1;
        BOOL enabled = TRUE;
        BOOL visible = TRUE;

        UINT count = engine->ReadCombo(
            engine->Context,
            controlId,
            buffer.data(),
            capacity,
            itemData.data(),
            &selection,
            text.data(),
            MaxControlText,
            &enabled,
            &visible);
        if (count == (UINT)-1)
            return snapshot;

        snapshot.Ok = true;
        snapshot.Selection = selection >= 0 && (UINT)selection < capacity
            ? selection : -1;
        snapshot.Text = text.data();
        snapshot.Enabled = enabled != FALSE;
        snapshot.Visible = visible != FALSE;
        for (UINT i = 0; i < count && i < capacity; i++)
        {
            snapshot.Items.emplace_back(
                buffer.data() + (size_t)i * K7_DIALOG_MIRROR_ITEM_TEXT);
            snapshot.ItemData.push_back(itemData[i]);
        }
        return snapshot;
    }

    struct TextSnapshot
    {
        std::wstring Text;
        bool Enabled = true;
        bool Visible = true;
        bool Ok = false;
    };

    inline TextSnapshot ReadText(
        const K7_DIALOG_MIRROR_ENGINE* engine,
        UINT controlId)
    {
        TextSnapshot snapshot;
        if (!engine || !engine->ReadText || !engine->Context)
            return snapshot;

        std::vector<wchar_t> text(MaxControlText, L'\0');
        BOOL enabled = TRUE;
        BOOL visible = TRUE;
        if (!engine->ReadText(
                engine->Context,
                controlId,
                text.data(),
                MaxControlText,
                &enabled,
                &visible))
        {
            return snapshot;
        }
        snapshot.Ok = true;
        snapshot.Text = text.data();
        snapshot.Enabled = enabled != FALSE;
        snapshot.Visible = visible != FALSE;
        return snapshot;
    }

    struct CheckSnapshot
    {
        bool Checked = false;
        bool Enabled = true;
        bool Visible = true;
        bool Ok = false;
    };

    inline CheckSnapshot ReadCheck(
        const K7_DIALOG_MIRROR_ENGINE* engine,
        UINT controlId)
    {
        CheckSnapshot snapshot;
        if (!engine || !engine->ReadCheck || !engine->Context)
            return snapshot;

        BOOL checked = FALSE;
        BOOL enabled = TRUE;
        BOOL visible = TRUE;
        if (!engine->ReadCheck(
                engine->Context,
                controlId,
                &checked,
                &enabled,
                &visible))
        {
            return snapshot;
        }
        snapshot.Ok = true;
        snapshot.Checked = checked != FALSE;
        snapshot.Enabled = enabled != FALSE;
        snapshot.Visible = visible != FALSE;
        return snapshot;
    }
}
