#include "CinderSPARCK.h"

using glm::vec2;
using glm::vec3;

CinderSPARCK::CinderSPARCK(std::string syphonServerName, uint32_t outputSide) {
    mOutputSide = outputSide;
	mOutputFbo = ci::gl::Fbo::create(6 * outputSide, outputSide);

	auto outputMesh = makeCubeMapToRowLayoutMesh_SPARCK(outputSide);
	auto outputShader = ci::gl::GlslProg::create(ci::app::loadAsset("OutputCubeMapToRect_v.glsl"), ci::app::loadAsset("OutputCubeMapToRect_f.glsl"));
	outputShader->uniform("uCubeMap", mCubeMapTextureBind);

	mOutputBatch = ci::gl::Batch::create(outputMesh, outputShader);

	mSyphonServer = ciSyphon::Server::create();
	mSyphonServer->setName(syphonServerName);
}

CinderSPARCK::sendCubeMap(ci::gl::TextureCubeMapRef cubeMap) {
    ci::gl::ScopedFramebuffer scpFbo(mOutputFbo);

    ci::gl::ScopedMatrices scpMat;
    ci::gl::setMatricesWindow(6 * mOutputSide, mOutputSide);
    gl::ScopedViewport scpView(0, 0, 6 * mOutputSide, mOutputSide);

    ci::gl::clear(Color(0, 0, 0));

    ci::gl::ScopedTextureBind scpTex(cubeMap, mCubeMapTextureBind);

    mOutputBatch->draw();

	// Publish to Syphon
	mSyphonServer->publishTexture(mOutputFbo->getColorTexture());
}

void genCubeMapFace(vec2 ul, vec2 lr, vec3 tu, vec3 tv, vec3 tw, vector<vec2> * positions, vector<vec3> * cubeMapTexCoords) {
	positions->emplace_back(lr.x, ul.y); // upper right
	cubeMapTexCoords->emplace_back(normalize(tw + tu + tv));

	positions->emplace_back(ul.x, ul.y); // upper left
	cubeMapTexCoords->emplace_back(normalize(tw + tv));

	positions->emplace_back(lr.x, lr.y); // lower right
	cubeMapTexCoords->emplace_back(normalize(tw + tu));

	positions->emplace_back(ul.x, lr.y); // lower left
	cubeMapTexCoords->emplace_back(normalize(tw));

	positions->emplace_back(lr.x, lr.y); // lower right
	cubeMapTexCoords->emplace_back(normalize(tw + tu));

	positions->emplace_back(ul.x, ul.y); // upper left
	cubeMapTexCoords->emplace_back(normalize(tw + tv));
}

// Special configuration for Martin Fröhlich's SPARCK program
// (SPARCK expects a rectangle of -X, +Z, +X, -Z, +Y, -Y)
ci::gl::VboMeshRef makeCubeMapToRowLayoutMesh_SPARCK(uint32_t side) {
	vector<vec2> positions;
	vector<vec3> cubeMapTexCoords;

	int place;
	// Generate six sets of positions and texcoords for drawing the six faces of the cube map
	// + X
	place = 2;
	genCubeMapFace(vec2(side * place, 0), vec2(side * (place + 1), side), vec3(0, 0, -2), vec3(0, 2, 0), vec3(1, -1, 1), & positions, & cubeMapTexCoords);
	// - X
	place = 0;
	genCubeMapFace(vec2(side * place, 0), vec2(side * (place + 1), side), vec3(0, 0, 2), vec3(0, 2, 0), vec3(-1, -1, -1), & positions, & cubeMapTexCoords);
	// + Y
	place = 4;
	genCubeMapFace(vec2(side * place, 0), vec2(side * (place + 1), side), vec3(2, 0, 0), vec3(0, 0, -2), vec3(-1, 1, 1), & positions, & cubeMapTexCoords);
	// - Y
	place = 5;
	genCubeMapFace(vec2(side * place, 0), vec2(side * (place + 1), side), vec3(2, 0, 0), vec3(0, 0, 2), vec3( -1, -1, -1 ), & positions, & cubeMapTexCoords);
	// Note: I think that somewhere in SPARCK the Z-axis is getting swapped.
	// But frames communicating with it can be fixed by swapping (or not)
	// the places of the +Z and -Z sides of the cubemap
	// + Z
	place = 1;
	genCubeMapFace(vec2(side * place, 0), vec2(side * (place + 1), side), vec3(2, 0, 0), vec3(0, 2, 0), vec3(-1, -1, 1), & positions, & cubeMapTexCoords);
	// - Z
	place = 3;
	genCubeMapFace(vec2(side * place, 0), vec2(side * (place + 1), side), vec3(-2, 0, 0), vec3(0, 2, 0), vec3(1, -1, -1), & positions, & cubeMapTexCoords);

	auto posBufLayout = ci::geom::BufferLayout({ ci::geom::AttribInfo(ci::geom::POSITION, 2, 0, 0) });
	auto posBuf = ci::gl::Vbo::create(GL_ARRAY_BUFFER, positions, GL_STREAM_DRAW);
	auto cubeMapTexBufLayout = ci::geom::BufferLayout({ ci::geom::AttribInfo(ci::geom::TEX_COORD_0, 3, 0, 0) });
	auto cubeMapTexBuf = ci::gl::Vbo::create(GL_ARRAY_BUFFER, cubeMapTexCoords, GL_STREAM_DRAW);

	return ci::gl::VboMesh::create(positions.size(), GL_TRIANGLES, { { posBufLayout, posBuf }, { cubeMapTexBufLayout, cubeMapTexBuf } });
}

