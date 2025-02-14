#include "world_init.hpp"
#include "../../tinyECS/registry.hpp"
#include <iostream>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();
//
//    GridLine& gridLine = registry.gridLines.emplace(entity);
//	gridLine.start_pos = start_pos;
//	gridLine.end_pos = end_pos;
//
//	registry.renderRequests.insert(
//		entity,
//		{
//			TEXTURE_ASSET_ID::TEXTURE_COUNT,
//			EFFECT_ASSET_ID::LINE,
//			GEOMETRY_BUFFER_ID::DEBUG_LINE
//		}
//	);
//
//	registry.colors.insert(entity, {0.0, 0.0, 1.0});

	return entity;
}




Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

//	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
//	registry.renderRequests.insert(
//		entity,
//		{
//			// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
//			TEXTURE_ASSET_ID::TEXTURE_COUNT,
//			EFFECT_ASSET_ID::LINE,
//			GEOMETRY_BUFFER_ID::DEBUG_LINE
//		}
//	);
//
//	// Create motion
//	Motion& motion = registry.motions.emplace(entity);
//	motion.angle = 0.f;
//	motion.velocity = { 0, 0 };
//	motion.position = position;
//	motion.scale = scale;

//	registry.debugComponents.emplace(entity);
	return entity;
}