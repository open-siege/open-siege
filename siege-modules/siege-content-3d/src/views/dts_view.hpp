#ifndef DTS_VIEW_HPP
#define DTS_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include "dts_controller.hpp"
#include "gl_renderer.hpp"

namespace siege::views
{
  struct opengl_deleter
  {
    void operator()(HGLRC gl_context)
    {
      assert(::wglDeleteContext(gl_context) == TRUE);
    }
  };

  struct dts_view : win32::window_ref
  {
    dts_controller controller;

    win32::static_control render_view;
    win32::list_box selection;

    std::optional<gl_renderer> renderer;

    std::map<win32::gdi_drawing_context_ref, win32::auto_handle<HGLRC, opengl_deleter>> contexts;

    dts_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto on_create(const win32::create_message&)
    {
      auto control_factory = win32::window_factory(ref());

      render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_OWNERDRAW });

      selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS,
      });

      selection.InsertString(-1, L"Palette 1");
      selection.InsertString(-1, L"Palette 2");
      selection.InsertString(-1, L"Palette 3");

      return 0;
    }

    auto on_destroy(win32::destroy_message)
    {
      ::wglMakeCurrent(nullptr, nullptr);

      return 0;
    }

    auto on_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (controller.is_shape(stream))
      {
        auto size = controller.load_shape(stream);

        if (size > 0)
        {
          ::InvalidateRect(render_view, nullptr, TRUE);
          return TRUE;
        }
      }

      return FALSE;
    }

    auto create_or_get_gl_context(win32::gdi_drawing_context_ref gdi_context)
    {
      auto existing_gl_context = contexts.find(gdi_context);

      if (existing_gl_context == contexts.end())
      {
        PIXELFORMATDESCRIPTOR pfd = {
          .nSize = sizeof(PIXELFORMATDESCRIPTOR),
          .nVersion = 1,
          .dwFlags = PFD_DRAW_TO_WINDOW |
                     PFD_SUPPORT_OPENGL,
          .iPixelType = PFD_TYPE_RGBA,
          .cColorBits = 32,
          .cAlphaBits = 8,
          .cDepthBits = 32
        };
        int iPixelFormat;

        iPixelFormat = ChoosePixelFormat(gdi_context, &pfd);

        assert(iPixelFormat != 0);
        assert(SetPixelFormat(gdi_context, iPixelFormat, &pfd) == TRUE);
        auto temp = std::make_pair(win32::gdi_drawing_context_ref(gdi_context.get()), win32::auto_handle<HGLRC, opengl_deleter>(::wglCreateContext(gdi_context)));

        assert(temp.first != nullptr);
        assert(temp.second != nullptr);
        existing_gl_context = contexts.emplace(std::move(temp)).first;
      }

      if (::wglMakeCurrent(existing_gl_context->first, existing_gl_context->second) == FALSE)
      {
      }

      return existing_gl_context;
    }

    auto on_size(win32::size_message sized)
    {
      auto left_size = SIZE{ .cx = (sized.client_size.cx / 3) * 2, .cy = sized.client_size.cy };
      auto right_size = SIZE{ .cx = sized.client_size.cx - left_size.cx, .cy = sized.client_size.cy };

      render_view.SetWindowPos(left_size);
      render_view.SetWindowPos(POINT{});

      win32::gdi_drawing_context gdi_context(render_view);
      auto context = create_or_get_gl_context(win32::gdi_drawing_context_ref(gdi_context.get()));
      glViewport(0, 0, left_size.cx, left_size.cy);

      selection.SetWindowPos(right_size);
      selection.SetWindowPos(POINT{ .x = left_size.cx });

      return 0;
    }

    auto on_control_color(win32::static_control_color_message message)
    {
      auto context = create_or_get_gl_context(win32::gdi_drawing_context_ref(message.context));
      glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      return (LRESULT)GetStockBrush(DC_BRUSH);
    }

    auto on_draw_item(win32::draw_item_message message)
    {
      if (message.item.hwndItem == render_view && message.item.itemAction == ODA_DRAWENTIRE)
      {
        auto existing_gl_context = create_or_get_gl_context(win32::gdi_drawing_context_ref(message.item.hDC));
        
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2i(0, 1);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2i(-1, -1);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex2i(1, -1);
        glEnd();
        glFlush();
      }

      return TRUE;
    }
  };
}// namespace siege::views

#endif