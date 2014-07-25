#include "LinkedList.h"

template<class _Ty, class T> LinkedList<_Ty, T>::LinkedList() {
	this->count = 0x00;
	this->head = this->last = nullptr;
}

template<class _Ty, class T> LinkedList<_Ty, T>::~LinkedList() {
	this->clear();
}

template<class _Ty, class T> void LinkedList<_Ty, T>::add(const _Ty& data) {
	Node* newNode = new Node();
	if(this->count == 0x00) { //empty list
		newNode->prev = nullptr;
		newNode->next = nullptr;
		this->head = this->last = newNode;
	} else {
		newNode->prev = this->last;
		this->last->next = newNode;
		this->last = newNode;
	}
	this->last->value = _Ty(data);
	this->count++;
}
		
template<class _Ty, class T> typename LinkedList<_Ty, T>::Node* LinkedList<_Ty, T>::getNode(const size_t pos) {
	size_t offset = 0x00;
	LinkedList<_Ty, T>::Node* tmpNode = this->head;
	while(tmpNode && offset != pos) {
		tmpNode = tmpNode->next;
		offset++;
	}
	return tmpNode;
}

template<class _Ty, class T>  _Ty& LinkedList<_Ty, T>::getValue(const size_t pos) {
	LinkedList<_Ty, T>::Node* tmpNode = this->getNode(pos);
	if(!tmpNode)
		throw std::exception();
	return tmpNode->getValue();
}

template<class _Ty, class T> void LinkedList<_Ty, T>::remove(const _Ty& data) {
	LinkedList<_Ty, T>::Node* tmpNode = this->head;
	size_t idx = 0x00;
	while(tmpNode) {
		if(tmpNode->value == data) {
			return this->removeAt(idx);
		}
		idx++;
		tmpNode = tmpNode->next;
	}
}

template<class _Ty, class T> void LinkedList<_Ty, T>::removeAt(const size_t pos) {
	if(this->count == 0x00 || pos >= this->count)
		return;

	LinkedList<_Ty, T>::Node* tmpNode = this->head;
	if(pos == 0x00) {
		if(this->count == 0x01) { //only header
			this->head = this->last = nullptr;
		} else { //other items are available -> create 
			this->head->next->prev = nullptr; 
			this->head = this->head->next;
		}
	} else { //something in between or last
		size_t offset = 0x00;
		while(tmpNode && offset != pos) {
			tmpNode = tmpNode->next;
			offset++;
		}
		if(tmpNode == this->last) {
			this->last = this->last->prev;
			this->last->next = nullptr;
		} else {
			tmpNode->prev->next = tmpNode->next;
			tmpNode->next->prev = tmpNode->prev;
		}
	}
	if(tmpNode) {
		this->count--;
		delete tmpNode;
		tmpNode = nullptr;
	}
}

template<class _Ty, class T> void LinkedList<_Ty, T>::clear() {
	size_t cnt = this->getNodeCount();
	for(unsigned int i=0;i<cnt;i++)
		this->removeAt(0x00);
}