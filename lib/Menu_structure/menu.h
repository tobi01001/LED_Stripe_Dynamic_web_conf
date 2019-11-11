#ifndef menu_h
#define menu_h
#include <Arduino.h>

enum fieldtypes {
    NumberFieldType,
    BooleanFieldType,
    SelectFieldType,
    ColorFieldType,
    TitleFieldType,
    SectionFieldType,
    InvalidFieldType
  };

class menu_item
{
    public:
        menu_item(const char* label, fieldtypes type, uint8_t position, menu_item* prev, menu_item* parent, menu_item* child)
        {
            this->_next = NULL;
            this->_prev = prev;
            if(prev != NULL)
            {
                prev->_next = this;
            }
            this->_parent   = parent;
            if(parent != NULL)
            {
                parent->_child = this;
            }
            this->_label    = label;
            this->_type     = type;
            this->_position = position;
        }
        inline menu_item*  getNext(void)      { if(_next == NULL) return _child;  return _next; }
        inline menu_item*  getPrev(void)      { if(_prev == NULL) return _parent; return _prev;}
        inline menu_item*  getParent(void)    { return _parent; }
        inline menu_item*  getChild(void)    { return _child; }
        inline uint8_t     getPosition(void)  { return _position; }
        inline const char* getLabel(void)     { return _label; }
        inline fieldtypes  getFieldType(void) { return _type; }

    private:
    menu_item* _next   = NULL;
    menu_item* _prev   = NULL;
    menu_item* _parent = NULL;
    menu_item* _child  = NULL;
    uint8_t     _position;
    const char* _label;
    fieldtypes  _type;
};


#endif // menu_h