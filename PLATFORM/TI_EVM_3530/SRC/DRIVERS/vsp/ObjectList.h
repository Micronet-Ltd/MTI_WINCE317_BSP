#include <windows.h>

class ObjectList
{
private:
	LPVOID *m_list;
	int m_maxObjects;

public:
	ObjectList(int maxObjects)
	{
		m_maxObjects = maxObjects;
		m_list = new LPVOID[m_maxObjects];
		for (int i = 0; i < m_maxObjects; i++)
		{
			m_list[i] = NULL;
		}
	}

	~ObjectList()
	{
		delete[] m_list;
	}

	BOOL Add(LPVOID object)
	{
		for (int i = 0; i < m_maxObjects; i++)
		{
			if (!m_list[i])
			{
				m_list[i] = object;
				return TRUE;
			}
		}
		return FALSE;
	}

	VOID Delete(LPVOID object)
	{
		for (int i = 0; i < m_maxObjects; i++)
		{
			if (m_list[i] == object)
			{
				m_list[i] = NULL;
				return;
			}
		}
	}

	LPVOID GetFirst()
	{
		for (int i = 0; i < m_maxObjects; i++)
		{
			if (m_list[i])
				return m_list[i];
		}
		return NULL;
	}
};