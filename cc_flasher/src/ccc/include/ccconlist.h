/***************************************************************************
 *   Copyright (C) 2005 by Peter Kuhar                                     *
 *   peter.kuhar{guest.arnes.si                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 #ifndef _CCCONLIST_H_
 #define _CCCONLIST_H_
 
 struct CCConListNode{
    CCConListNode *prev;
    CCConListNode *next;
    void *ptr;
 };

 class CCConList
 {
 public:
    CCConList(){
        count= 0;
        first= 0;
        current= 0;
        last= 0;
    };
    ~CCConList(){
        DeleteAllItems();  
    }
    CCConListNode *first;
    CCConListNode *current;
    CCConListNode *last;
    int  count;
    bool GetNextItem(void **pptr){ 
        if(!count || !current)
            return false;
        if(!current->next)
            return false;
        current = current->next;
        *pptr = current->ptr;
        return true;   
    }
	bool GetNextItem(CCConListNode *&pPtr, void **pptr){ 
        if(!count || !pPtr)
            return false;
        if(!pPtr->next)
            return false;
        pPtr = pPtr->next;
        *pptr = pPtr->ptr;
        return true;   
    }

    bool GetPrevItem(void **pptr){
        if(!count || !current)
            return false;
        if(!current->prev)
            return false;
        current = current->prev;
        *pptr = current->ptr;
        return true;
    }
    bool GetFirstItem(void **pptr){
        if(count == 0)
            return false;
        current = first;
        *pptr = current->ptr;
        return true;
    }
	bool GetFirstItem(CCConListNode *&pPtr,void **pptr){
        if(count == 0)
            return false;
        pPtr = first;
        *pptr = pPtr->ptr;
        return true;
    }

    bool GetCurrentItem(void **pptr){
        if(count == 0 || !current)
            return false;
        *pptr = current->ptr;
        return true;
    }
    bool AddItem(void *ptr){
        if(count == 0){
            first= new CCConListNode;
            first->prev = 0;
            first->next = 0;
            first->ptr = ptr; 
            current = first;
            last = first;
        }else{
            last->next = new CCConListNode;
            last->next->next=0;
            last->next->prev = last;
            last->next->ptr = ptr;
            current = last->next;
            last = current;
        }
        count ++;
        return true;
    }
    bool DeleteCurrentItem(){
        if(count == 0 || !current)
            return false;
        if(current->prev)
            current->prev->next = current->next;
        if(current->next)
            current->next->prev = current->prev;
        if(current == first)
            first = current->next;
        if(current == last)
            last = current->prev;
        CCConListNode *tmp;
        tmp=current;
        current = current->next;
        delete tmp;
        count--;
        return true;
    }
    void DeleteAllItems(){
        current=first;
        while(DeleteCurrentItem());
    }
 };

 #endif // _CCCONLIST_H_
