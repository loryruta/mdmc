#pragma once

#include <string>

#include <GL/glew.h> // #error:  gl.h included before glew.h

#include "phase.hpp"

#include "util/fixed_target_camera.hpp"

#include <tsl/htrie_map.h>

#include "minecraft/minecraft_world.hpp"
#include "minecraft/minecraft_renderer.hpp"

#include "minecraft/voxelizer/minecraft_baked_block_pool.hpp"
#include "minecraft/voxelizer/minecraft_block_voxelizer.hpp"

#include "octree/octree.hpp"
#include "octree/octree_builder.hpp"
#include "octree/octree_tracer.hpp"

#include <imgui.h>

class MinecraftAssetsPhase : public Phase
{
public:
	enum class State
	{
		SELECT_MINECRAFT_VERSION,
		VOXELIZATION_OPTIONS,
		VIEW
	};

private:
	State state = State::SELECT_MINECRAFT_VERSION;

	MinecraftBlockVoxelizer minecraft_block_voxelizer;
	OctreeBuilder octree_builder;
	OctreeRenderer octree_tracer;

	unsigned int resolution = 1;

	bool view_block_octree = false;
	std::shared_ptr<Octree> octree;
	void test_view_block_octree_input(GLFWwindow* window, float delta);

	std::string version = "1.15.2";

	std::shared_ptr<MinecraftAssets const> assets;
	std::shared_ptr<MinecraftContext const> context;

	//std::vector<std::pair<std::string, MinecraftBlockStateVariant const*>> block_by_id;
	int current_block_id = 0;
	void shift_block_id(bool forward);

	double last_block_change_time = 0;

	FixedTargetCamera camera;
	void test_camera_input(GLFWwindow* window, float delta);
	void test_block_sliding_input(GLFWwindow* window, float delta);

	std::shared_ptr<MinecraftBakedBlockPool> minecraft_baked_block_pool;
	MinecraftRenderer minecraft_renderer;

	void setup(std::string const& version);

	void ui_menu_bar(unsigned int& y);

	void ui_select_minecraft_version(std::string& current_version, const std::function<void(const std::string&)>& on_load);
	void ui_voxelization_options();

	void ui_block_info(unsigned int& y);
	void ui_camera_info(unsigned int& y);

	void ui_main();

public:
	MinecraftAssetsPhase();

	void update(Stage& stage, float delta) override;
	void render(Stage& stage) override;
};