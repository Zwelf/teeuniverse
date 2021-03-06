/*
 * Copyright (C) 2016 necropotame (necropotame@gmail.com)
 * 
 * This file is part of TeeUniverse.
 * 
 * TeeUniverse is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * TeeUniverse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with TeeUniverse.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SHARED_LOCALIZATION__
#define __SHARED_LOCALIZATION__

#include <shared/kernel.h>
#include <shared/tl/hashtable.h>

#include <unicode/ucnv.h>
#include <unicode/numfmt.h>
#include <unicode/upluralrules.h>
#include <unicode/tmutfmt.h>

#include <stdarg.h>

class CLocalizableString
{
public:
	enum
	{
		TYPE_STRING=0,
		TYPE_INTEGER,
		TYPE_FLOAT,
		TYPE_SECONDS,
		TYPE_NONE,
	};
	
	struct CParameter 
	{
		union
		{
			struct
			{
				char* m_pValue;
			} m_String;
			struct
			{
				int m_Value;
			} m_Integer;
			struct
			{
				float m_Value;
			} m_Float;
			struct
			{
				float m_Value;
			} m_Seconds;
		};
		dynamic_string m_Name;
		int m_Type;
		
		CParameter()
		{
			m_Type = TYPE_NONE;
		}
		
		CParameter(const CParameter& Param)
		{
			m_Type = TYPE_NONE;
			*this = Param;
		}
		
		CParameter(CParameter&& Param)
		{
			m_Type = TYPE_NONE;
			*this = std::move(Param);
		}
		
		~CParameter()
		{
			if(m_Type == TYPE_STRING && m_String.m_pValue)
				delete[] m_String.m_pValue;
		}
		
		inline void operator=(const CParameter& Param)
		{			
			m_Name = Param.m_Name;
			
			if(m_Type == TYPE_STRING)
			{
				if(m_String.m_pValue)
					delete[] m_String.m_pValue;
			}
			
			m_Type = Param.m_Type;
			if(Param.m_Type == TYPE_STRING)
			{
				int Length = str_length(Param.m_String.m_pValue);
				m_String.m_pValue = new char[Length+1];
				str_copy(m_String.m_pValue, Param.m_String.m_pValue, Length+1);
			}
			else
				m_String = Param.m_String;
		}
		
		inline void operator=(CParameter&& Param)
		{
			m_String = Param.m_String;
			m_Type = Param.m_Type;
			m_Name = std::move(Param.m_Name);
			
			if(Param.m_Type == TYPE_STRING)
				Param.m_String.m_pValue = NULL;
		}
	};
	
protected:
	dynamic_string m_Text;
	std::vector< CParameter > m_Parameters;

public:
	CLocalizableString()
	{ }
	
	CLocalizableString(const char* pText)
	{
		m_Text = pText;
	}
	
	~CLocalizableString()
	{
		
	}
	
	inline const char* GetText() const { return m_Text.buffer(); }
	inline const std::vector< CParameter >& GetParameters() const { return m_Parameters; }
	
	inline void AddInteger(const char* pName, int Value)
	{
		m_Parameters.emplace_back();
		m_Parameters.back().m_Name = pName;
		m_Parameters.back().m_Integer.m_Value = Value;
		m_Parameters.back().m_Type = TYPE_INTEGER;
	}
	
	inline void AddFloat(const char* pName, float Value)
	{
		m_Parameters.emplace_back();
		m_Parameters.back().m_Name = pName;
		m_Parameters.back().m_Float.m_Value = Value;
		m_Parameters.back().m_Type = TYPE_FLOAT;
	}
	
	inline void AddString(const char* pName, const char* pValue)
	{
		m_Parameters.emplace_back();
		m_Parameters.back().m_Name = pName;
		int Length = str_length(pValue);
		m_Parameters.back().m_String.m_pValue = new char[Length+1];
		str_copy(m_Parameters.back().m_String.m_pValue, pValue, Length+1);
		m_Parameters.back().m_Type = TYPE_STRING;
	}
	
	inline void ClearParameters()
	{
		m_Parameters.clear();
	}
};

#define _(TEXT) (TEXT)
#define _LSTRING(TEXT) (CLocalizableString(TEXT))

class CLocalization : public CSharedKernel::CComponent
{
public:
	enum
	{
		PLURALTYPE_NONE=0,
		PLURALTYPE_ZERO,
		PLURALTYPE_ONE,
		PLURALTYPE_TWO,
		PLURALTYPE_FEW,
		PLURALTYPE_MANY,
		PLURALTYPE_OTHER,
		NUM_PLURALTYPES,
	};

	class IListener
	{
	public:
		virtual void OnLocalizationModified() = 0;
	};
	
	class CLanguage
	{
	protected:
		class CEntry
		{
		public:
			char* m_apVersions[NUM_PLURALTYPES];
			
			CEntry()
			{
				for(int i=0; i<NUM_PLURALTYPES; i++)
					m_apVersions[i] = NULL;
			}
			
			void Free()
			{
				for(int i=0; i<NUM_PLURALTYPES; i++)
					if(m_apVersions[i])
						delete[] m_apVersions[i];
			}
		};
		
	protected:
		char m_aName[64];
		char m_aFilename[64];
		char m_aParentFilename[64];
		bool m_Loaded;
		int m_Direction;
		
		hashtable< CEntry, 128 > m_Translations;
	
	public:
		UPluralRules* m_pPluralRules;
		UNumberFormat* m_pNumberFormater;
		UNumberFormat* m_pPercentFormater;
		icu::TimeUnitFormat* m_pTimeUnitFormater;
		
	public:
		CLanguage();
		CLanguage(const char* pName, const char* pFilename, const char* pParentFilename);
		~CLanguage();
		
		inline const char* GetParentFilename() const { return m_aParentFilename; }
		inline const char* GetFilename() const { return m_aFilename; }
		inline const char* GetName() const { return m_aName; }
		inline int GetWritingDirection() const { return m_Direction; }
		inline void SetWritingDirection(int Direction) { m_Direction = Direction; }
		inline bool IsLoaded() const { return m_Loaded; }
		bool Load(CStorage* pStorage);
		const char* Localize(const char* pKey) const;
		const char* Localize_P(int Number, const char* pText) const;
	};
	
	enum
	{
		DIRECTION_LTR=0,
		DIRECTION_RTL,
		NUM_DIRECTIONS,
	};

protected:
	CLanguage* m_pMainLanguage;
	std::vector<IListener*> m_pListeners;
	bool m_UpdateListeners;
	
	UConverter* m_pUtf8Converter;

public:
	std::vector< std::unique_ptr<CLanguage> > m_pLanguages;
	dynamic_string m_Cfg_MainLanguage;

protected:
	const char* LocalizeWithDepth(const char* pLanguageCode, const char* pText, int Depth);
	const char* LocalizeWithDepth_P(const char* pLanguageCode, int Number, const char* pText, int Depth);
	
	void AppendInteger(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, int Number);
	void AppendFloat(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, float Number);
	void AppendPercent(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, double Number);
	void AppendDuration(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, int Number, icu::TimeUnit::UTimeUnitFields Type);
	
public:
	CLocalization(CSharedKernel* pKernel);
	virtual ~CLocalization();
	
	virtual bool InitConfig(int argc, const char** argv);
	virtual void SaveConfig(class CCLI_Output* pOutput);
	virtual bool Init();
	virtual bool PreUpdate();
	
	void AddListener(IListener* pListener);
	void RemoveListener(IListener* pListener);
	
	inline bool GetWritingDirection() const { return (!m_pMainLanguage ? DIRECTION_LTR : m_pMainLanguage->GetWritingDirection()); }
	
	//localize
	const char* Localize(const char* pLanguageCode, const char* pText);
	//localize and find the appropriate plural form based on Number
	const char* Localize_P(const char* pLanguageCode, int Number, const char* pText);
	
	//format
	void Format(dynamic_string& Buffer, const char* pLanguageCode, const CLocalizableString& LString);
	void FormatInteger(dynamic_string& Buffer, const char* pLanguageCode, int Number);
	
	int ParseInteger(const char* pLanguageCode, const char* pText);
	float ParseFloat(const char* pLanguageCode, const char* pText);
	float ParsePercent(const char* pLanguageCode, const char* pText);
};

#endif
