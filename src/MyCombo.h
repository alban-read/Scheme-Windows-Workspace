//////////////////////////////////////////////////////
// MyCombo.h

#ifndef MYCOMBO_H
#define MYCOMBO_H



// Declaration of the CMyCombo class
class CMyCombo : public c_combo_box_ex
{
public:
	CMyCombo();
	virtual ~CMyCombo();
	BOOL add_items();
	virtual void OnDestroy();
	void SetImages(int nImages, UINT ImageID);

protected:
	virtual void PreCreate(CREATESTRUCT& cs);

private:
	CImageList m_imlImages;
};


#endif // MYCOMBO_H
