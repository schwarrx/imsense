/*
 * app.h
 *
 *  Created on: Jun 3, 2019
 *      Author: nelaturi
 */

#ifndef APP_H_
#define APP_H_

#include "core.h"

class App : public NVPWindow {
public:
	virtual bool init();
	virtual void display();
	virtual void reshape(int w, int h);
	virtual void motion(int x, int y, int dx, int dy);
	virtual void keyboardchar(unsigned char key, int mods, int x, int y);
	virtual void mouse (NVPWindow::MouseButton button, NVPWindow::ButtonAction state, int mods, int x, int y);

	void		LoadRAW(char* fname, Vector3DI res, int bpp);
	bool		ConvertToFloat(Vector3DI res, uchar* dat);
	void		Rebuild() { Rebuild(m_VolMax, m_sparse, m_halo); }
	void		Rebuild( Vector3DF vmax, bool bSparse, bool bHalo);
	void        setVolumeParams();
	void        setCameraParams();

	void 		initGVDB();
	void		start_guis(int w, int h);
	void		draw_topology();

	Vector3DI	m_DataRes;
	int			m_DataBpp;			// 1=byte, 2=ushort, 4=float
	char*		m_DataBuf;
	Vector3DF	m_VolMax;

	int			m_gvdb_tex;
	int			mouse_down;
	bool		m_show_topo;
	bool		m_sparse;
	bool		m_halo;
};


#endif /* APP_H_ */
