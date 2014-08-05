#pragma once

#ifndef __CUSTOM_LINKED_LIST__
#define __CUSTOM_LINKED_LIST__

template<class _Ty, class T = typename std::enable_if<std::has_trivial_copy<_Ty>::value && std::has_trivial_assign<_Ty>::value>::type> class LinkedList {	
	public:
		struct Node {
			friend class LinkedList;
			private:
				_Ty value;
				Node* prev;
				Node* next;
			public:
				__inline _Ty getValue() const { return this->value; }
				__inline const _Ty& getValueCONST() const { return this->value; }
				__inline Node* getNextNode() const { return this->next; }
				__inline Node* getPreviousNode() const { return this->prev; }
		};
	private:
		Node* head;
		Node* last;
		size_t count;
	public:
		LinkedList();
		~LinkedList();

		__inline Node* getHeadNode() const { return this->head; }
		__inline const size_t getNodeCount() const { return this->count; }
		
		void add(const _Ty& data);
		Node* getNode(const size_t pos);
		_Ty& getValue(const size_t pos);

		Node* remove(const Node* node);
		Node* remove(const _Ty& data);
		Node* removeAt(const size_t pos);
		void clear();
};

#endif //__CUSTOM_LINKED_LIST__