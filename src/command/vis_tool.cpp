// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "command.h"

#include "../include/aegisub/context.h"
#include "../libresrc/libresrc.h"
#include "../project.h"
#include "../video_display.h"
#include "../visual_tool_clip.h"
#include "../visual_tool_cross.h"
#include "../visual_tool_drag.h"
#include "../visual_tool_perspective.h"
#include "../visual_tool_rotatexy.h"
#include "../visual_tool_rotatez.h"
#include "../visual_tool_scale.h"
#include "../visual_tool_vector_clip.h"

#include <libaegisub/make_unique.h>

namespace {
	using cmd::Command;

	template<class T>
	struct visual_tool_command : public Command {
		CMD_TYPE(COMMAND_VALIDATE | COMMAND_RADIO)

		bool Validate(const agi::Context *c) override {
			return !!c->project->VideoProvider();
		}

		bool IsActive(const agi::Context *c) override {
			return c->videoDisplay->ToolIsType(typeid(T));
		}

		void operator()(agi::Context *c) override {
			c->videoDisplay->SetTool(agi::make_unique<T>(c->videoDisplay, c));
		}
	};

	template<VisualToolVectorClipMode M>
	struct visual_tool_vclip_command : public Command {
		CMD_TYPE(COMMAND_VALIDATE | COMMAND_RADIO)

		bool Validate(const agi::Context *c) override {
			return !!c->project->VideoProvider();
		}

		bool IsActive(const agi::Context *c) override {
			return c->videoDisplay->ToolIsType(typeid(VisualToolVectorClip)) && c->videoDisplay->GetSubTool() == M;
		}

		void operator()(agi::Context *c) override {
			c->videoDisplay->SetTool(agi::make_unique<VisualToolVectorClip>(c->videoDisplay, c));
			c->videoDisplay->SetSubTool(M);
		}
	};

	template<VisualToolPerspectiveSetting M>
	struct visual_tool_persp_setting : public Command {
		CMD_TYPE(COMMAND_VALIDATE | COMMAND_TOGGLE)

		bool Validate(const agi::Context *c) override {
			return c->videoDisplay->ToolIsType(typeid(VisualToolPerspective));
		}

		bool IsActive(const agi::Context *c) override {
			return Validate(c) && c->videoDisplay->GetSubTool() & M;
		}

		void operator()(agi::Context *c) override {
			if (!c->videoDisplay->ToolIsType(typeid(VisualToolPerspective)))
				c->videoDisplay->SetTool(agi::make_unique<VisualToolPerspective>(c->videoDisplay, c));
			c->videoDisplay->SetSubTool(c->videoDisplay->GetSubTool() ^ M);
		}
	};

	struct visual_mode_cross final : public visual_tool_command<VisualToolCross> {
		CMD_NAME("video/tool/cross")
		CMD_ICON(visual_standard)
		STR_MENU("Standard")
		STR_DISP("Standard")
		STR_HELP("Standard mode, double click sets position")
	};

	struct visual_mode_drag final : public visual_tool_command<VisualToolDrag> {
		CMD_NAME("video/tool/drag")
		CMD_ICON(visual_move)
		STR_MENU("Drag")
		STR_DISP("Drag")
		STR_HELP("Drag subtitles")
	};

	struct visual_mode_rotate_z final : public visual_tool_command<VisualToolRotateZ> {
		CMD_NAME("video/tool/rotate/z")
		CMD_ICON(visual_rotatez)
		STR_MENU("Rotate Z")
		STR_DISP("Rotate Z")
		STR_HELP("Rotate subtitles on their Z axis")
	};

	struct visual_mode_rotate_xy final : public visual_tool_command<VisualToolRotateXY> {
		CMD_NAME("video/tool/rotate/xy")
		CMD_ICON(visual_rotatexy)
		STR_MENU("Rotate XY")
		STR_DISP("Rotate XY")
		STR_HELP("Rotate subtitles on their X and Y axes")
	};

	struct visual_mode_perspective final : public visual_tool_command<VisualToolPerspective> {
		CMD_NAME("video/tool/perspective")
		CMD_ICON(visual_vector_clip_drag) //TODO
		STR_MENU("Apply 3D Perspective")
		STR_DISP("Apply 3D Perspective")
		STR_HELP("Rotate and shear subtitles to make them fit a given quad's perspective")
	};

	struct visual_mode_scale final : public visual_tool_command<VisualToolScale> {
		CMD_NAME("video/tool/scale")
		CMD_ICON(visual_scale)
		STR_MENU("Scale")
		STR_DISP("Scale")
		STR_HELP("Scale subtitles on X and Y axes")
	};

	struct visual_mode_clip final : public visual_tool_command<VisualToolClip> {
		CMD_NAME("video/tool/clip")
		CMD_ICON(visual_clip)
		STR_MENU("Clip")
		STR_DISP("Clip")
		STR_HELP("Clip subtitles to a rectangle")
	};

	struct visual_mode_vector_clip final : public visual_tool_command<VisualToolVectorClip> {
		CMD_NAME("video/tool/vector_clip")
		CMD_ICON(visual_vector_clip)
		STR_MENU("Vector Clip")
		STR_DISP("Vector Clip")
		STR_HELP("Clip subtitles to a vectorial area")
	};

	// Perspective settings
	struct visual_mode_perspective_plane final : public visual_tool_persp_setting<PERSP_PLANE> {
		CMD_NAME("video/tool/perspective/plane")
		CMD_ICON(visual_clip)
		STR_MENU("Show Surrounding Plane")
		STR_DISP("Show Surrounding Plane")
		STR_HELP("Toggles showing a second quad for the ambient 3D plane.")
	};

	struct visual_mode_perspective_grid final : public visual_tool_persp_setting<PERSP_GRID> {
		CMD_NAME("video/tool/perspective/grid")
		CMD_ICON(visual_rotatexy)
		STR_MENU("Show Grid")
		STR_DISP("Show Grid")
		STR_HELP("Toggles showing a 3D grid in the visual perspective tool")
	};

	// Vector clip tools

	struct visual_mode_vclip_drag final : public visual_tool_vclip_command<VCLIP_DRAG> {
		CMD_NAME("video/tool/vclip/drag")
		CMD_ICON(visual_vector_clip_drag)
		STR_MENU("Drag")
		STR_DISP("Drag")
		STR_HELP("Drag control points")
	};

	struct visual_mode_vclip_line final : public visual_tool_vclip_command<VCLIP_LINE> {
		CMD_NAME("video/tool/vclip/line")
		CMD_ICON(visual_vector_clip_line)
		STR_MENU("Line")
		STR_DISP("Line")
		STR_HELP("Appends a line")
	};
	struct visual_mode_vclip_bicubic final : public visual_tool_vclip_command<VCLIP_BICUBIC> {
		CMD_NAME("video/tool/vclip/bicubic")
		CMD_ICON(visual_vector_clip_bicubic)
		STR_MENU("Bicubic")
		STR_DISP("Bicubic")
		STR_HELP("Appends a bezier bicubic curve")
	};
	struct visual_mode_vclip_convert final : public visual_tool_vclip_command<VCLIP_CONVERT> {
		CMD_NAME("video/tool/vclip/convert")
		CMD_ICON(visual_vector_clip_convert)
		STR_MENU("Convert")
		STR_DISP("Convert")
		STR_HELP("Converts a segment between line and bicubic")
	};
	struct visual_mode_vclip_insert final : public visual_tool_vclip_command<VCLIP_INSERT> {
		CMD_NAME("video/tool/vclip/insert")
		CMD_ICON(visual_vector_clip_insert)
		STR_MENU("Insert")
		STR_DISP("Insert")
		STR_HELP("Inserts a control point")
	};
	struct visual_mode_vclip_remove final : public visual_tool_vclip_command<VCLIP_REMOVE> {
		CMD_NAME("video/tool/vclip/remove")
		CMD_ICON(visual_vector_clip_remove)
		STR_MENU("Remove")
		STR_DISP("Remove")
		STR_HELP("Removes a control point")
	};
	struct visual_mode_vclip_freehand final : public visual_tool_vclip_command<VCLIP_FREEHAND> {
		CMD_NAME("video/tool/vclip/freehand")
		CMD_ICON(visual_vector_clip_freehand)
		STR_MENU("Freehand")
		STR_DISP("Freehand")
		STR_HELP("Draws a freehand shape")
	};
	struct visual_mode_vclip_freehand_smooth final : public visual_tool_vclip_command<VCLIP_FREEHAND_SMOOTH> {
		CMD_NAME("video/tool/vclip/freehand_smooth")
		CMD_ICON(visual_vector_clip_freehand_smooth)
		STR_MENU("Freehand smooth")
		STR_DISP("Freehand smooth")
		STR_HELP("Draws a smoothed freehand shape")
	};
}

namespace cmd {
	void init_visual_tools() {
		reg(agi::make_unique<visual_mode_cross>());
		reg(agi::make_unique<visual_mode_drag>());
		reg(agi::make_unique<visual_mode_rotate_z>());
		reg(agi::make_unique<visual_mode_rotate_xy>());
		reg(agi::make_unique<visual_mode_perspective>());
		reg(agi::make_unique<visual_mode_scale>());
		reg(agi::make_unique<visual_mode_clip>());
		reg(agi::make_unique<visual_mode_vector_clip>());

		reg(agi::make_unique<visual_mode_perspective_plane>());
		reg(agi::make_unique<visual_mode_perspective_grid>());

		reg(agi::make_unique<visual_mode_vclip_drag>());
		reg(agi::make_unique<visual_mode_vclip_line>());
		reg(agi::make_unique<visual_mode_vclip_bicubic>());
		reg(agi::make_unique<visual_mode_vclip_convert>());
		reg(agi::make_unique<visual_mode_vclip_insert>());
		reg(agi::make_unique<visual_mode_vclip_remove>());
		reg(agi::make_unique<visual_mode_vclip_freehand>());
		reg(agi::make_unique<visual_mode_vclip_freehand_smooth>());
	}
}
