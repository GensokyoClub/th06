// these functions were moved here because of a
// circular dependency between GfxInterface and ZunMath headers

// ZunMath uses the GfxBackend instance to manipulate viewport and depth range
// while GfxInterface uses ZunMatrix for SetTransformMatrix

// (unfortunately this also means that inverseViewportMatrix is no longer inlined)

#include "ZunMath.hpp"
#include "GameWindow.hpp"

void ZunViewport::Set() const
{
    g_GfxBackend->SetViewport(this->x * WIDTH_RESOLUTION_SCALE + VIEWPORT_OFF_X,
                              (GAME_WINDOW_HEIGHT_REAL - ((this->y + this->height) * HEIGHT_RESOLUTION_SCALE)) -
                                  VIEWPORT_OFF_Y,
                              this->width * WIDTH_RESOLUTION_SCALE, this->height * HEIGHT_RESOLUTION_SCALE);
    g_GfxBackend->SetDepthRange(this->minZ, this->maxZ);
}

void ZunViewport::Get()
{
    u32 viewPortGet[4];
    f32 depthRangeGet[2];

    g_GfxBackend->GetViewport(viewPortGet);
    g_GfxBackend->GetDepthRange(depthRangeGet);

    this->x = (viewPortGet[0] - VIEWPORT_OFF_X) / WIDTH_RESOLUTION_SCALE;
    this->y = (viewPortGet[1] - VIEWPORT_OFF_Y) / HEIGHT_RESOLUTION_SCALE;
    this->width = viewPortGet[2] / WIDTH_RESOLUTION_SCALE;
    this->height = viewPortGet[3] / HEIGHT_RESOLUTION_SCALE;
    this->minZ = depthRangeGet[0];
    this->maxZ = depthRangeGet[1];

    // Convert from OpenGL to D3D conventions
    this->y = GAME_WINDOW_HEIGHT - (this->y + this->height);
}

// Returns a matrix that maps screen coordinates to NDCs. Used for drawing RHW positions,
//   since D3D interprets them has having been already transformed, but OpenGL has no option
//   to prevent transformation

ZunMatrix inverseViewportMatrix()
{
    ZunMatrix inverseMatrix;
    ZunViewport viewport;

    viewport.Get();

    inverseMatrix.Identity();

    // Mappings:
    //   X: [viewport x .. viewport width] -> [-1 .. 1]
    //   Y: [viewport y .. viewport height] -> [1 .. -1] (Axis inverted since NDCs are cartesian)
    //   Z: [0 .. 1] -> [-1 .. 1]. D3D does NOT interpolate this value using the viewport's depth range!
    //                             Therefore we must change our depth range to [0.0 .. 1.0] as well

    // One difference between OpenGL and D3D is that in D3D, pixels are centered on integers, whereas
    //   in OpenGL, they're on half-integer coordinates. Originally, this function finished with a glTranslatef
    //   call to account for this, but OpenGL seems to be very finicky with rasterizing edges on pixel centers,
    //   and most positions in EoSD do use whole integer coordinates for edges (D3D seems to be less
    //   finicky about rasterization). To prevent obvious off-by-one errors with edges in the UI, no accounting
    //   is done for the pixel coordinate discrepancy aside from changing the rounding in DrawOrthographic, if
    //   applied, to use whole integers (OpenGL pixel boundaries), rather than half integers (D3D pixel boundaries).
    //   Graphical output should really be checked thoroughly to make sure nothing (especially in the 3D draw functions)
    //   ends up a half pixel off.

    inverseMatrix.Translate(-1.0f, 1.0f, -1.0f);
    inverseMatrix.Scale(1.0f / (viewport.width / 2.0f), -1.0f / (viewport.height / 2.0f), 2.0f);
    inverseMatrix.Translate(-viewport.x, -viewport.y, 0.0f);

    g_GfxBackend->SetDepthRange(0.0f, 1.0f);

    return inverseMatrix;
}