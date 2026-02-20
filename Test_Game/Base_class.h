#pragma once
#include "Header.h"
#include "Engine.h"
#include "Network.h"

enum Class_id {
	BASE_CLASS = 0,
	PLAYER_CLASS,
	ENEMY_CLASS
};


class Base_class
{
public:
	Base_class(int object_id, Class_id classID = BASE_CLASS);
	Base_class()=delete;
	virtual ~Base_class();

	static std::vector<Base_class*>& Get_all_classes();
	static Base_class* Get_class_by_id(Class_id& class_id, int id);
	static Base_class* Create_object(Class_id class_id, int id);

	int m_object_id;
	Position m_position;

	static void Replicate_all_classes();
protected:
	static void Replicate_certain_class();
	static void Replicate_classes_id();
	static void Replicate_object_pos();

	static int m_current_object_id;
	static std::vector<Base_class*> m_all_objects;
	static std::unordered_map<int, Class_id> m_class_id_map;

	int m_rotation;
};
