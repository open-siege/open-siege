#ifndef WIN32_REBAR_HPP
#define WIN32_REBAR_HPP

#include <siege/platform/win/desktop/win32_window.hpp>
#include <CommCtrl.h>

namespace win32
{
    struct rebar : window
    {
        using window::window;
        constexpr static auto class_Name = REBARCLASSNAMEW;

        [[nodiscard]] inline std::optional<RECT> GetRect(wparam_t index)
        {
            RECT result;

            if (SendMessageW(self, RB_GETRECT, index, std::bit_cast<lparam_t>(&result)))
            {
                return result;
            }

            return std::nullopt;
        }

        [[nodiscard]] inline std::uint32_t GetBarHeight()
        {
            return std::uint32_t(SendMessageW(self, RB_GETBARHEIGHT, 0, 0));
        }

        [[nodiscard]] inline lparam_t GetBandCount()
        {
            return SendMessageW(self, RB_GETBANDCOUNT, 0, 0);
        }

        [[maybe_unused]] inline void SetBandWidth(wparam_t index, lparam_t new_width)
        {
            SendMessageW(self, RB_SETBANDWIDTH, index, new_width);
        }

        [[maybe_unused]] inline void MaximizeBand(wparam_t index, lparam_t ideal_width = 0)
        {
            SendMessageW(self, RB_MAXIMIZEBAND, index, ideal_width);
        }

        [[maybe_unused]] inline std::optional<REBARBANDINFOW> GetBandChildSize(wparam_t index)
        {
            REBARBANDINFOW band {.cbSize = sizeof(REBARBANDINFOW), .fMask = RBBIM_CHILDSIZE};
            
            if (SendMessageW(self, RB_GETBANDINFOW, index, std::bit_cast<win32::lparam_t>(&band)))
            {
                return band;
            }

            return std::nullopt;
        }

        [[maybe_unused]] inline bool SetBandInfo(wparam_t index, REBARBANDINFOW band)
        {
            band.cbSize = sizeof(band);
            return SendMessageW(self, RB_SETBANDINFOW, 
                index, std::bit_cast<win32::lparam_t>(&band));
        }

        [[maybe_unused]] inline bool InsertBand(wparam_t position, REBARBANDINFOW band)
        {
            band.cbSize = sizeof(band);

            bool mask_not_set = band.fMask == 0;

            if (mask_not_set && band.hwndChild)
            {
                band.fMask |= RBBIM_CHILD;
            }

            if (mask_not_set && band.lpText)
            {
                band.fMask |= RBBIM_TEXT;
            }

            if (mask_not_set && (
                band.cxMinChild ||
                band.cyMinChild ||
                band.cyChild ||
                band.cyMaxChild
                ))
            {
                band.fMask |= RBBIM_CHILDSIZE;
            }

            if (mask_not_set && band.cx)
            {
                band.fMask |= RBBIM_SIZE;
            }

            if (mask_not_set && band.cxIdeal)
            {
                band.fMask |= RBBIM_IDEALSIZE;
            }

            if (mask_not_set && band.cxHeader)
            {
                band.fMask |= RBBIM_HEADERSIZE;
            }

            if (mask_not_set && band.fStyle)
            {
                band.fMask |= RBBIM_STYLE;
            }
            
            return SendMessageW(self, RB_INSERTBANDW, 
                position, std::bit_cast<win32::lparam_t>(&band));
        }
    };
}

#endif