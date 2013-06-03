#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#include <utility>

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

struct class_tree_node_type;
typedef class_tree_node_type *class_tree_node;

struct class_method_type;
typedef class_method_type *class_method;

class Type
{
	private:
	class_tree_node node;

	public:
	Type( class_tree_node n = NULL);
	Type( Type tn)
	{
		node = tn.node;
	}

	bool operator bool() const
	{
		return node->is_defined();
	}

	bool operator->() const
	{
		return node;
	}

	friend class_tree_node &operator=( class_tree_node &p, const Type &);
};

struct class_method_type
{
	private:
	Type type;
	class_method next;

	public:
	class_method_type( Type nt, class_method nn = NULL) : type( nt), next( nn) {}

	Type hd() const { return type;}
	class_method tl() const { return next;}

	void set_hd( Type nt) { type = nt;}
	void set_tl( class_method nn) { next = nn;}

	bool is_defined() const { return type; }
};

typedef SymbolTable< Symbol, class_tree_node_type> symtable_type;
typedef SymbolTable< Symbol, class_method_type> method_table_type;

// Env vars.
extern method_table_type *method_table;

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;

  symtable_type symtable;
  symtable_type vartable;

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
};

struct class_tree_node_type {
	class_tree_node set_head;
	int set_rank;
	int set_size;

	class_tree_node father;
	class_tree_node son;
	class_tree_node sibling;
	Class_ contain;
	int depth;

	method_table_type method_table;

	class_tree_node find_set()
	{
		return set_head == this ? this : set_head = set_head->find_set();
	}

	public:
	friend class_tree_node union_set( class_tree_node, class_tree_node);

	class_tree_node_type( Class_ class_) :
		set_head( this), set_rank( 0), set_size( 1),
		father( NULL), son( NULL), sibling( NULL),
		contain( class_), depth( 0)
	{
		method_table.enterscope();
		if ( class_)
		{
			this->set_contain(class_);
		}
	}

	~class_tree_node_type()
	{
		method_table.exitscope();
	}

	bool set_father( class_tree_node father)
	{
		this->father = father;
		this->depth = father->depth + 1;
		this->sibling = father->son;
		father->son = this;

		return union_set( this, father);
	}

	void set_contain( Class_ contain)
	{
		this->contain = contain;
		::method_table = &this->method_table;
		return contain->collect_Methods();
	}

	bool is_subtype_of( const class_tree_node_type *super) const
	{
		if ( !is_defined() || !super->is_defined())
		{
			return false;
		}

		const class_tree_node_type *leg = this;
		while ( leg->depth > super->depth)
		{
			leg = leg->father;
		}

		return leg == super;
	}

	bool is_defined() const
	{
		return contain && contain != Null_type;
	}

	bool operator bool() const
	{
		return is_defined();
	}

	class_method find_method( Symbol name)
	{
		class_method ret = method_table.lookup( name);
		return ret ? ret : ( father != this ? father->find_method( name) : NULL);
	}

	friend class_tree_node find_lca( class_tree_node, class_tree_node);

	bool walk_down();
};

#endif

