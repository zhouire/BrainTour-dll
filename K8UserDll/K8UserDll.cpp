// K8UserDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "K8UserDll.hpp"

#include <GL/freeglut.h>

static void drawCube(float size)
{
	float h = size * 0.5f;
	glBegin(GL_QUADS);
	glVertex3f(-h, -h, -h);
	glVertex3f(-h, +h, -h);
	glVertex3f(+h, +h, -h);
	glVertex3f(+h, -h, -h);
	glVertex3f(-h, -h, +h);
	glVertex3f(-h, +h, +h);
	glVertex3f(+h, +h, +h);
	glVertex3f(+h, -h, +h);
	glVertex3f(-h, -h, -h);
	glVertex3f(-h, +h, -h);
	glVertex3f(-h, +h, +h);
	glVertex3f(-h, -h, +h);
	glVertex3f(+h, -h, -h);
	glVertex3f(+h, +h, -h);
	glVertex3f(+h, +h, +h);
	glVertex3f(+h, -h, +h);
	glVertex3f(-h, -h, -h);
	glVertex3f(-h, -h, +h);
	glVertex3f(+h, -h, +h);
	glVertex3f(+h, -h, -h);
	glVertex3f(-h, +h, -h);
	glVertex3f(-h, +h, +h);
	glVertex3f(+h, +h, +h);
	glVertex3f(+h, +h, -h);
	glEnd();
}

static void init_matrix(float *matrix)
{
	int i;
	for (i = 0; i < 16; i++) matrix[i] = ((i / 4) == (i % 4)) ? 1.0f : 0.0f; // identity
}


static int mouse_old_x, mouse_old_y;
static int mouse_buttons = 0;
static bool auto_rotate_view = false;
static float view_matrix[16];
static float model_matrix[16];


namespace K8UserProxy {
	APIBundle *proxy;
	K8USERDLL_API int OnInit(APIBundle *p) {
		proxy = p;
		init_matrix(view_matrix);
		init_matrix(model_matrix);
		return 0;
	}

	// the modelview matrix set here affects volume rendering
	K8USERDLL_API void SetModelview() {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf(view_matrix);
		glMultMatrixf(model_matrix);
	}

	// in addition to the modelview matrix,
	// viewpoint position must be properly set for the volume rendering to work
	K8USERDLL_API void update_viewpoint()
	{
		float *dist = proxy->GetObjectDistance();
		float *zscale = proxy->GetZScale();
		float *viewpoint = proxy->GetViewpoint();

		SetModelview();
		float matrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		float view[3] = { matrix[2], matrix[6], matrix[10] };
		view[2] /= (*zscale);
		for (int i = 0; i < 3; ++i) {
			// (volume center) + (unit view direction) x (distance to the volume center)
			// where distance (*dist) is normalized by the longest dimension of the volume (ObjectScale)
			viewpoint[i] = proxy->VolumeSize[i] / 2 + view[i] * (*dist) * proxy->ObjectScale;
		}
	}

	// callback on mouse click
	K8USERDLL_API void mouse(int button, int state, int x, int y)
	{
		if (state == GLUT_DOWN)
		{
			mouse_buttons |= 1 << button;
		}
		else if (state == GLUT_UP)
		{
			mouse_buttons = 0;
		}

		mouse_old_x = x;
		mouse_old_y = y;
	}

	// callback on mouse drag
	K8USERDLL_API void motion(int x, int y)
	{
		float dx = (float)(x - mouse_old_x);
		float dy = (float)(y - mouse_old_y);

		if (mouse_buttons & 1) {
			float *angle = proxy->GetViewAngle();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glRotatef(dy * 0.2f * (*angle) / 60.0f, 1, 0, 0);
			glRotatef(dx * 0.2f * (*angle) / 60.0f, 0, 1, 0);
			glMultMatrixf(model_matrix);
			glGetFloatv(GL_MODELVIEW_MATRIX, model_matrix);
		}
		else if (mouse_buttons & 4) {
			float *dist = proxy->GetObjectDistance();
			*dist += dy * 0.001f;
			if (*dist > proxy->ObjectDistanceMax) *dist = proxy->ObjectDistanceMax;
			if (*dist < proxy->ObjectDistanceMin) *dist = proxy->ObjectDistanceMin;
		}

		update_viewpoint();

		mouse_old_x = x;
		mouse_old_y = y;
	}

	// callback on mouse wheel motion
	K8USERDLL_API void OnScale(int s, float speed)
	{
		float *dist = proxy->GetObjectDistance();
		if (s > 0) {
			*dist /= speed;
			if (*dist < proxy->ObjectDistanceMin) *dist = proxy->ObjectDistanceMin;
		}
		else if (s < 0) {
			*dist *= speed;
			if (*dist > proxy->ObjectDistanceMax) *dist = proxy->ObjectDistanceMax;
		}
	}

	// callback on keyboard input
	K8USERDLL_API void keyboard(unsigned char key, int x, int y)
	{
		switch (key) {
		case 'e':
			auto_rotate_view = !auto_rotate_view;
			break;
		case 'w':
			glutReshapeWindow(proxy->window_width, proxy->window_height);
			break;
		case '0':
			glutPositionWindow(0, 0);
			break;
		case 'i':
		{
			init_matrix(view_matrix);
			init_matrix(model_matrix);
			float *dist = proxy->GetObjectDistance();
			*dist = proxy->ObjectDistanceInit;
		}
			break;
		case 'f':
			glutFullScreen();
			break;
		case (27):
			glutDestroyWindow(glutGetWindow());
			return;
		default:
			break;
		}
		glutPostRedisplay();
	}

	// display callback, called after volume rendering
	// (can't seem to get user-defined rendering work before volume rendering...)
	K8USERDLL_API void DrawPostVolumeRendering()
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		float *angle = proxy->GetViewAngle();
		gluPerspective(*angle, proxy->window_width / (float)proxy->window_height, 0.01, 10.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		float *dist = proxy->GetObjectDistance();
		glTranslatef(0, 0, -(*dist));
		glMultMatrixf(view_matrix);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);

		glColor3f(0.5f, 0.5f, 0.5f);
		glBegin(GL_LINES);
		int i, j;
		for (i = 0; i < 10; i++)
		{
			for (j = 0; j < 10; j++)
			{
				glVertex3f((0.0f - 4.5f) * 0.1f, (j - 4.5f) * 0.1f, 0);
				glVertex3f((9.0f - 4.5f) * 0.1f, (j - 4.5f) * 0.1f, 0);
				glVertex3f((i - 4.5f) * 0.1f, (0.0f - 4.5f) * 0.1f, 0);
				glVertex3f((i - 4.5f) * 0.1f, (9.0f - 4.5f) * 0.1f, 0);
			}
		}
		glEnd();

		// objects above stay with the world coordinates
		glMultMatrixf(model_matrix);
		// objects below stay with the volume coordinates

		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 0; i < 10; i++)
		{
			for (j = 0; j < 10; j++)
			{
				glPushMatrix();
				glTranslatef((i - 4.5f) * 0.1f, (j - 4.5f) * 0.1f, 0);
				drawCube(0.01f);
				glPopMatrix();
			}
		}

		float scale = 1.0f / proxy->ObjectScale; // this maps volume size in voxels to world/object coordinates
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		float w = 0.5f * proxy->VolumeSize[0] * scale; // (half) width
		float h = 0.5f * proxy->VolumeSize[1] * scale; // (half) height
		float d = 0.5f * proxy->VolumeSize[2] * scale; // (half) depth
		glVertex3f(-w, -h, -d); glVertex3f(+w, -h, -d);
		glVertex3f(-w, +h, -d); glVertex3f(+w, +h, -d);
		glVertex3f(-w, -h, +d); glVertex3f(+w, -h, +d);
		glVertex3f(-w, +h, +d); glVertex3f(+w, +h, +d);

		glVertex3f(-w, -h, -d); glVertex3f(-w, +h, -d);
		glVertex3f(+w, -h, -d); glVertex3f(+w, +h, -d);
		glVertex3f(-w, -h, +d); glVertex3f(-w, +h, +d);
		glVertex3f(+w, -h, +d); glVertex3f(+w, +h, +d);

		glVertex3f(-w, -h, -d); glVertex3f(-w, -h, +d);
		glVertex3f(+w, -h, -d); glVertex3f(+w, -h, +d);
		glVertex3f(-w, +h, -d); glVertex3f(-w, +h, +d);
		glVertex3f(+w, +h, -d); glVertex3f(+w, +h, +d);
		glEnd();

		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	// idle callback for updating the view when there is no user input
	// return true when there is actually an update
	K8USERDLL_API bool idle()
	{
		if (auto_rotate_view)
		{
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMultMatrixf(view_matrix);
			glRotatef(1.0f, 0, 1, 0);
			glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);
			return true;
		}
		else
		{
			return false;
		}
	}
}
