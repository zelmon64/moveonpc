#pragma once
#include "MovePrecompiled.h"

class MOVE_EXPORT CIniFile
{

public:
	struct Record
	{
		std::string Comments;
		char Commented;
		std::string Section;
		std::string Key;
		std::string Value;
	};

	enum CommentChar
	{
		Pound = '#',
		SemiColon = ';'
	};

	CIniFile(void);
	virtual ~CIniFile(void);

	static bool AddSection(std::string SectionName, std::string FileName);
	static bool CommentRecord(CommentChar cc, std::string KeyName,std::string SectionName,std::string FileName);
	static bool CommentSection(char CommentChar, std::string SectionName, std::string FileName);
	static std::string Content(std::string FileName);
	static bool Create(std::string FileName);
	static bool DeleteRecord(std::string KeyName, std::string SectionName, std::string FileName);
	static bool DeleteSection(std::string SectionName, std::string FileName);
	static std::vector<Record> GetRecord(std::string KeyName, std::string SectionName, std::string FileName);
	static std::vector<Record> GetSection(std::string SectionName, std::string FileName);
	static std::vector<std::string> GetSectionNames(std::string FileName);
	static std::string GetValue(std::string KeyName, std::string SectionName, std::string FileName);
	static bool RecordExists(std::string KeyName, std::string SectionName, std::string FileName);
	static bool RenameSection(std::string OldSectionName, std::string NewSectionName, std::string FileName);
	static bool SectionExists(std::string SectionName, std::string FileName);
	static bool SetRecordComments(std::string Comments, std::string KeyName, std::string SectionName, std::string FileName);
	static bool SetSectionComments(std::string Comments, std::string SectionName, std::string FileName);
	static bool SetValue(std::string KeyName, std::string Value, std::string SectionName, std::string FileName);
	static bool Sort(std::string FileName, bool Descending);
	static bool UnCommentRecord(std::string KeyName,std::string SectionName,std::string FileName);
	static bool UnCommentSection(std::string SectionName, std::string FileName);

	static inline float toF(std::string value)
	{
		std::istringstream iss(value);
		float retval;
		iss >> retval;
		return retval;
	}

	static inline float toI(std::string value)
	{
		std::istringstream iss(value);
		int retval;
		iss >> retval;
		return retval;
	}

	static int GetInt(std::string KeyName, std::string SectionName, std::string FileName)
	{
		std::string value = GetValue(KeyName, SectionName, FileName);
		return toI(value);
	}

	static int GetFloat(std::string KeyName, std::string SectionName, std::string FileName)
	{
		std::string value = GetValue(KeyName, SectionName, FileName);
		return toF(value);
	}

	static Move::Vector3 GetVector3(std::string KeyName, std::string SectionName, std::string FileName)
	{
		using namespace std;
		std::string value = GetValue(KeyName, SectionName, FileName);
		std::vector<std::string> tokens;
		std::istringstream iss(value);
		copy(istream_iterator<std::string>(iss),
				 istream_iterator<std::string>(),
				 back_inserter<vector<std::string> >(tokens));
		return Move::Vector3(toF(tokens[0]),toF(tokens[1]),toF(tokens[2]));
	}

	static Move::Matrix3 GetMatrix3(std::string KeyName, std::string SectionName, std::string FileName)
	{
		using namespace std;
		std::string value = GetValue(KeyName, SectionName, FileName);
		vector<std::string> tokens;
		std::istringstream iss(value);
		copy(istream_iterator<std::string>(iss),
				 istream_iterator<std::string>(),
				 back_inserter<vector<std::string> >(tokens));
		return Move::Matrix3(toF(tokens[0]),toF(tokens[1]),toF(tokens[2]),
					   toF(tokens[3]),toF(tokens[4]),toF(tokens[5]),
					   toF(tokens[6]),toF(tokens[7]),toF(tokens[8]));
	}

	static bool SetValue(std::string KeyName, int Value, std::string SectionName, std::string FileName)
	{
		using namespace std;
		std::string str;
		std::stringstream stream;
        stream  << Value;
        str = stream.str();
		return SetValue(KeyName, str, SectionName, FileName);
	}

	static bool SetValue(std::string KeyName, float Value, std::string SectionName, std::string FileName)
	{
		using namespace std;
		std::string str;
		std::stringstream stream;
        stream  << Value;
        str = stream.str();
		return SetValue(KeyName, str, SectionName, FileName);
	}

	static bool SetValue(std::string KeyName, Move::Vector3 Value, std::string SectionName, std::string FileName)
	{
		using namespace std;
		std::string str;
		std::stringstream stream;
        stream  << Value.x << " " << Value.y << " " << Value.z;
        str = stream.str();
		return SetValue(KeyName, str, SectionName, FileName);
	}

	static bool SetValue(std::string KeyName, Move::Quaternion Value, std::string SectionName, std::string FileName)
	{
		using namespace std;
		std::string str;
		std::stringstream stream;
        stream  << Value.w << " " << Value.x << " " << Value.y << " " << Value.z;
        str = stream.str();
		return SetValue(KeyName, str, SectionName, FileName);
	}

	static bool SetValue(std::string KeyName, Move::Matrix3 Value, std::string SectionName, std::string FileName)
	{
		using namespace std;
		std::string str;
		std::stringstream stream;
        stream << Value[0][0] << " " << Value[0][1] << " " << Value[0][2]
			   << Value[1][0] << " " << Value[1][1] << " " << Value[1][2]
			   << Value[2][0] << " " << Value[2][1] << " " << Value[2][2];
        str = stream.str();
		return SetValue(KeyName, str, SectionName, FileName);
	}

private:
	static std::string Trim(std::string str);
	static std::vector<Record> GetSections(std::string FileName);
	static bool Load(std::string FileName, std::vector<Record>& content);	
	static bool Save(std::string FileName, std::vector<Record>& content);

	struct RecordSectionIs : std::unary_function<Record, bool>
	{
		std::string section_;

		RecordSectionIs(const std::string& section): section_(section){}

		bool operator()( const Record& rec ) const
		{
			return rec.Section == section_;
		}
	};

	struct RecordSectionKeyIs : std::unary_function<Record, bool>
	{
		std::string section_;
		std::string key_;

		RecordSectionKeyIs(const std::string& section, const std::string& key): section_(section),key_(key){}

		bool operator()( const Record& rec ) const
		{
			return ((rec.Section == section_)&&(rec.Key == key_));
		}
	};

	struct AscendingSectionSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Section < End.Section;
		}
	};

	struct DescendingSectionSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Section > End.Section;
		}
	};

	struct AscendingRecordSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Key < End.Key;
		}
	};

	struct DescendingRecordSort
	{
		bool operator()(Record& Start, Record& End)
		{
			return Start.Key > End.Key;
		}
	};
};