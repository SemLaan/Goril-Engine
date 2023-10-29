#include "camera.h"


Camera CameraCreateOrthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    Camera camera = {};
    camera.projection = mat4_orthographic(left, right, bottom, top, near, far);
    camera.view = mat4_identity();
    camera.projectionView = camera.projection;
    camera.position = (vec3){};
    camera.rotation = (vec3){};
    camera.dirty = false;

    return camera;
}

mat4 CameraGetProjectionView(Camera* camera)
{
    if (camera->dirty)
    {
        mat4 rotation = mat4_rotate_xyz(camera->rotation);
        mat4 translation = mat4_translate(camera->position);
        camera->view = mat4_mul_mat4(rotation, translation);
        camera->projectionView = mat4_mul_mat4(camera->projection, camera->view);
        camera->dirty = false;
    }

    return camera->projectionView;
}

vec3 CameraGetPosition(Camera* camera)
{
    return camera->position;
}

void CameraSetPosition(Camera* camera, vec3 position)
{
    camera->position = position;
    camera->dirty = true;
}

vec3 CameraGetRotation(Camera* camera)
{
    return camera->rotation;
}

void CameraSetRotation(Camera* camera, vec3 rotation)
{
    camera->rotation = rotation;
    camera->dirty = true;
}
