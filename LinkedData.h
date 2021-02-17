#pragma once

template<class T>
class LinkedData {
	LinkedData* next;
	LinkedData* prev;
public:
	bool empty;
	LinkedData() { empty = true; };
	LinkedData(T* value) { data = reinterpret_cast<T*>(value); empty = false; };
	void Add(T* value)
	{
		if (empty == true)
		{
			data = reinterpret_cast<T*>(value);
			empty = false;
		}
		else
		{
			if (next == nullptr)
			{
				next = new LinkedData<T>(value);
				next->prev = this;
			}
			else
				next->Add(value);
		}
	};
	void AddOrdered(T* value, float order)
	{
		if (empty == true)
		{
			data = reinterpret_cast<T*>(value);
			empty = false;
			return;
		}

		LinkedData<T> newNode = new LinkedData<T>(value);
		//put this reference after the previous value
		if (prev->order < order) 
		{
			if (next == nullptr)
			{
				next = newNode;
				next->prev = this;
			}
			else
				next->Add(value);
		}
		//assign the previous value to the current one
		else 
		{
			prev->prev = newNode; //remove association to previous
			newNode.next = prev; //attach previous to next 
		}

		newNode->order = order;
	};

	std::wstring ToString() 
	{
		if (next != nullptr)
			return std::to_wstring(*data) + L" " + next->ToString();

		return std::to_wstring(*data);
	}
private:
	T* data;
	float order;
};