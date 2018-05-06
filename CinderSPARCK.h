// Cinder-Syphon is required for sending frames over the wire.
// There are many forks of it, but this one is mine: https://github.com/mhintz/Cinder-Syphon
#include "Syphon.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/GeomIo.h"

class CinderSPARCK {
    public:
        CinderSPARCK();

        void sendCubeMap(ci::gl::TextureCubeMapRef);
    private:
        ci::gl::FboRef mOutputFbo;
        uint32_t mOutputSide;
        uint8_t mCubeMapTextureBind = 0;
        ci::gl::BatchRef mOutputBatch;
        ciSyphon::ServerRef mSyphonServer;
};

ci::gl::VboMeshRef makeCubeMapToRowLayoutMesh_SPARCK(uint32_t side);
