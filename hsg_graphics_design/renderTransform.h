#pragma once
#include <glm/glm.hpp>


class CustomRenderTranslation{
public:
	struct RenderTransformation
	{
		glm::vec3 Scale;
		glm::vec3 Rotation;
		glm::vec3 Translation;
	};
	void setRenderTransformation(RenderTransformation &_obj)
	{
		m_nRenderTransformationObj = _obj;
	}
	RenderTransformation getRenderTransformation()
	{
		return m_nRenderTransformationObj;
	}
	//����scale����
	void setScale(glm::vec3 &_scale)
	{
		m_nRenderTransformationObj.Scale = _scale;
	}
	void setScaleX(float &fval)
	{
		m_nRenderTransformationObj.Scale.x = fval;
	}
	void setScaleY(float &fval)
	{
		m_nRenderTransformationObj.Scale.y = fval;
	}
	void setScaleZ(float &fval)
	{
		m_nRenderTransformationObj.Scale.z = fval;
	}
	//����ratation����
	void setRotation(glm::vec3 &_rotation)
	{
		m_nRenderTransformationObj.Rotation = _rotation;
	}
	void setRotationX(float &_fval)
	{
		m_nRenderTransformationObj.Rotation.x = _fval;
	}
	void setRotationY(float &_fval)
	{
		m_nRenderTransformationObj.Rotation.y = _fval;
	}
	void setRotationZ(float &_fval)
	{
		m_nRenderTransformationObj.Rotation.z = _fval;
	}
	//����translation����
	void setTranslation(glm::vec3 &_translation)
	{
		m_nRenderTransformationObj.Translation = _translation;
	}
	void setTranslationX(float &_fval)
	{
		m_nRenderTransformationObj.Translation.x = _fval;
	}
	void setTranslationY(float &_fval)
	{
		m_nRenderTransformationObj.Translation.y = _fval;
	}
	void setTranslationZ(float &_fval)
	{
		m_nRenderTransformationObj.Translation.z = _fval;
	}

	//��ȡscale
	glm::vec3 getScale()
	{
		return m_nRenderTransformationObj.Scale;
	}
	float getScaleX()
	{
		return m_nRenderTransformationObj.Scale.x;
	}
	float getScaleY()
	{
		return m_nRenderTransformationObj.Scale.y;
	}
	float getScaleZ()
	{
		return m_nRenderTransformationObj.Scale.z;
	}
	//��ȡrotation
	glm::vec3 getRotation()
	{
		return m_nRenderTransformationObj.Rotation;
	}
	float getRotationX()
	{
		return m_nRenderTransformationObj.Rotation.x;
	}
	float getRotationY()
	{
		return m_nRenderTransformationObj.Rotation.y;
	}
	float getRotationZ()
	{
		return m_nRenderTransformationObj.Rotation.z;
	}
	//��ȡtranslation
	glm::vec3 getTraslation()
	{
		return m_nRenderTransformationObj.Translation;
	}
	float getTranslationX()
	{
		return m_nRenderTransformationObj.Translation.x;
	}
	float getTranslationY()
	{
		return m_nRenderTransformationObj.Translation.y;
	}
	float getTranslationZ()
	{
		return m_nRenderTransformationObj.Translation.z;
	}

private:
	RenderTransformation m_nRenderTransformationObj;
};
