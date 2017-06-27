#pragma once

////////////////////////////////////////////////////////////////////////////////
// The MIT License (MIT)
//
// Copyright (c) 2017 Nicholas Frechette & Animation Compression Library contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "acl/compression/animation_clip.h"
#include "acl/compression/skeleton.h"
#include "acl/core/memory.h"
#include "acl/core/error.h"

#include <stdint.h>

namespace acl
{
	inline void write_acl_clip(const RigidSkeleton& skeleton, const AnimationClip& clip, const char* acl_filename)
	{
		if (ACL_TRY_ASSERT(acl_filename != nullptr, "'acl_filename' cannot be NULL!"))
			return;

		std::FILE* file = nullptr;
		fopen_s(&file, acl_filename, "w");

		if (ACL_TRY_ASSERT(file != nullptr, "Failed to open ACL file for writing: %s", acl_filename))
			return;

		fprintf(file, "version = %u\n", 1);

		fprintf(file, "clip =\n");
		fprintf(file, "{\n");
		fprintf(file, "\tname = \"%s\"\n", clip.get_name().c_str());
		fprintf(file, "\tnum_samples = %u\n", clip.get_num_samples());
		fprintf(file, "\tsample_rate = %u\n", clip.get_sample_rate());
		fprintf(file, "\terror_threshold = %f\n", clip.get_error_threshold());
		fprintf(file, "}\n");

		fprintf(file, "bones =\n");
		fprintf(file, "[\n");
		uint16_t num_bones = skeleton.get_num_bones();
		for (uint16_t bone_index = 0; bone_index < num_bones; ++bone_index)
		{
			const RigidBone& bone = skeleton.get_bone(bone_index);
			const RigidBone& parent_bone = bone.is_root() ? bone : skeleton.get_bone(bone.parent_index);

			fprintf(file, "\t{\n");
			fprintf(file, "\t\tname = \"%s\"\n", bone.name.c_str());
			fprintf(file, "\t\tparent = \"%s\"\n", bone.is_root() ? "" : parent_bone.name.c_str());
			fprintf(file, "\t\tvertex_distance = %f\n", bone.vertex_distance);
			if (!quat_near_identity(bone.bind_rotation))
				fprintf(file, "\t\tbind_rotation = [ %.16f, %.16f, %.16f, %.16f ]\n", quat_get_x(bone.bind_rotation), quat_get_y(bone.bind_rotation), quat_get_z(bone.bind_rotation), quat_get_w(bone.bind_rotation));
			if (!vector_near_equal3(bone.bind_translation, vector_zero_64()))
				fprintf(file, "\t\tbind_translation = [ %.16f, %.16f, %.16f ]\n", vector_get_x(bone.bind_translation), vector_get_y(bone.bind_translation), vector_get_z(bone.bind_translation));
			//fprintf(file, "\t\tbind_scale = [ %.16f, %.16f, %.16f ]\n", vector_get_x(bone.bind_scale), vector_get_y(bone.bind_scale), vector_get_z(bone.bind_scale));
			fprintf(file, "\t}\n");
		}
		fprintf(file, "]\n");

		fprintf(file, "tracks =\n");
		fprintf(file, "[\n");
		for (uint16_t bone_index = 0; bone_index < num_bones; ++bone_index)
		{
			const RigidBone& rigid_bone = skeleton.get_bone(bone_index);
			const AnimatedBone& bone = clip.get_animated_bone(bone_index);

			fprintf(file, "\t{\n");
			fprintf(file, "\t\tname = \"%s\"\n", rigid_bone.name.c_str());
			fprintf(file, "\t\trotations =\n");
			fprintf(file, "\t\t[\n");
			uint32_t num_rotation_samples = bone.rotation_track.get_num_samples();
			for (uint32_t sample_index = 0; sample_index < num_rotation_samples; ++sample_index)
			{
				Quat_64 rotation = bone.rotation_track.get_sample(sample_index);
				fprintf(file, "\t\t\t[ %.16f, %.16f, %.16f, %.16f ]\n", quat_get_x(rotation), quat_get_y(rotation), quat_get_z(rotation), quat_get_w(rotation));
			}
			fprintf(file, "\t\t]\n");
			fprintf(file, "\t\ttranslations =\n");
			fprintf(file, "\t\t[\n");
			uint32_t num_translation_samples = bone.translation_track.get_num_samples();
			for (uint32_t sample_index = 0; sample_index < num_translation_samples; ++sample_index)
			{
				Vector4_64 translation = bone.translation_track.get_sample(sample_index);
				fprintf(file, "\t\t\t[ %.16f, %.16f, %.16f ]\n", vector_get_x(translation), vector_get_y(translation), vector_get_z(translation));
			}
			fprintf(file, "\t\t]\n");
			fprintf(file, "\t}\n");
		}
		fprintf(file, "]\n");

		std::fclose(file);
	}
}
