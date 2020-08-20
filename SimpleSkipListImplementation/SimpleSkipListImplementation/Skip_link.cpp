#include "../../../source/repos/std_lib_facilities.h"
#include<random>

// Exercise 11. Implement a skip list. This is like a layered linked list. 
// Each layer contains a reduced list of the elements from its lower layer.
// New elements are introduced to the base layer. A "coin toss" is used to determine whether it'll be included in the next layer up. This is repeated until it fails.
// The first item in the list is available at all layers and exclusively on the top layer.
// The main functions of a skip list are "Insert / Remove / Find / Get Link"

// Structure: Each Link has four pointers (Left / Right / Up / Down) And holds information on its value and position in terms of its layer and index.

// Skip_list class is currently simply represented as a link class with pointers. There are memory leaks which occur as a result of the delete function.
// I hope to resolve these leaks through better memory management techniques once covered in later chapters.

inline int rand_int(int min, int max) {
	static default_random_engine ran;
	return uniform_int_distribution<>{min, max}(ran);
}

int constructed = 0;
int destructed = 0;

class Skip_link {
public:
	Skip_link() { idx = 0, lyr = 0; ++constructed; }
	Skip_link(string value) : vl{ value } { idx = 0, lyr = 0; ++constructed; }
	~Skip_link() {
		++destructed;
	}

	Skip_link* left() const { return left_link; }
	Skip_link* right() const { return right_link; }
	Skip_link* up() const { return up_link; }
	Skip_link* down() const { return down_link; }

	void set_left(Skip_link* sl) { left_link = sl; }
	void set_right(Skip_link* sl) { right_link = sl; }
	void set_up(Skip_link* sl) { up_link = sl; }
	void set_down(Skip_link* sl) { down_link = sl; }


	Skip_link* insert(Skip_link*);
	Skip_link* remove(Skip_link*);
	Skip_link* find(string);
	Skip_link* get_link(int index);

	int get_index() { return idx; }
	int get_layer() { return lyr; }
	string get_value() { return vl; }
	void increment_index() { ++idx; }
	void decrement_index() { --idx; }

private:
	Skip_link(string value, int index, int layer, Skip_link* left = nullptr, Skip_link* right = nullptr, Skip_link* up = nullptr, Skip_link* down = nullptr) :
		vl{ value }, idx{ index }, lyr{ layer },
		left_link{ left }, right_link{ right }, up_link{ up }, down_link{ down } {++constructed; }

	void set_index(int index) { idx = index; }
	void set_layer(int layer) { lyr = layer; }

	string vl;
	int idx;
	int lyr;
	Skip_link* left_link = nullptr;
	Skip_link* right_link = nullptr;
	Skip_link* up_link = nullptr;
	Skip_link* down_link = nullptr;
};

Skip_link* get_top_link(Skip_link* sl) {
	while (sl->up()) { // set ll to the highest layer
		sl = sl->up();
	}
	return sl;
}

// Item inserted into base list. If it is the first item in the list, added to all layers (and also to its own new 'top layer' if required). 
//Otherwise coin toss at each layer to determine if it's included until it isn't (or until the highest layer is reached) DON'T include in the highest layer unless it's the first item.rt link and returns head
Skip_link* Skip_link::insert(Skip_link* sl) {

	if (sl == nullptr) { return this; } // Return current head incase of invalid insertion
	if (this == nullptr) {  // If sl is the first skip link set its indices to zero and return it
		idx = 0;
		lyr = 0;
		return sl;
	}

	if (this->get_value() == sl->get_value()) { // Return this if s1's value is the same as the current head
		return this;
	}

	if (this->get_value() > sl->get_value()) { // If the new value is lower than the current head, then update everything

		//Set links for the base level
		this->set_left(sl);
		sl->set_right(this);

		Skip_link* level = this;
		Skip_link* new_level_head = sl;

		// Go through each layer and set the new head.
		while (level) {


			Skip_link* line = level;

			// Go through each line on a layer and increment its index.
			while (line) {
				line->increment_index();
				line = line->right();
			}


			level = level->up();

			// If there is a already a level at this layer, add the new skiplink and set necessary links.
			if (level) {
				Skip_link* new_level_head_replacer = new Skip_link{ sl->get_value(), 0,level->get_layer() };

				level->set_left(new_level_head_replacer);
				new_level_head_replacer->set_right(level);
				new_level_head->set_up(new_level_head_replacer);
				new_level_head_replacer->set_down(new_level_head);
				new_level_head = new_level_head_replacer;

			}
			// Otherwise add the new head to a the new layer
			else {
				Skip_link* new_level_head_top = new Skip_link{ sl->get_value(), 0,new_level_head->get_layer() + 1 };
				new_level_head_top->set_down(new_level_head);
				new_level_head->set_up(new_level_head_top);
			}
		}
		return sl;
	}

	Skip_link* ll = this; //lower limit

	ll = get_top_link(ll);// set ll to the highest layer

	Skip_link* top_left = ll; //Keep track of top left position
	Skip_link* layer = ll; // Keep track of layer start
	Skip_link* cursor = ll; // Keep track of line position within layer

	while (layer) {
		while (cursor->right()) {
			if (cursor->right()->get_value() == sl->get_value()) { // Return this if s1's value has already been inserted.
				return this;
			}
			if (cursor->right()->get_value() > sl->get_value()) { // If the next value is higher then lower and upper limits have been found for the layer.
				ll = cursor;
				break;
			}
			cursor = cursor->right();
		}
		layer = ll->down();

		if (layer) {
			cursor = layer;
			ll = layer;
		}
	}

	// Increment indicies
	Skip_link* right_of_insertion = ll->right();

	while (right_of_insertion) {

		cursor = right_of_insertion;

		while (cursor) {
			cursor->increment_index();
			cursor = cursor->right();
		}
		while (!right_of_insertion->up()) {
			if (!right_of_insertion->right()) {
				break;
			}
			right_of_insertion = right_of_insertion->right();
		}
		right_of_insertion = right_of_insertion->up();
	}

	// Set the index of the new link as one higher than its left link
	int new_index = ll->get_index() + 1;
	sl->set_index(new_index);


	// Set links for 0-layer
	Skip_link* ul = ll->right();
	ll->set_right(sl);

	if (ul) {
		ul->set_left(sl);
		sl->set_right(ul);
	}

	sl->set_left(ll);

	if (!top_left->get_layer()) { //Add a new head link if top left is still zero layer.
		Skip_link* head_link = new Skip_link(this->get_value());
		head_link->set_layer(1);
		head_link->set_down(top_left);
		top_left->set_up(head_link);
		top_left = head_link;
	}

	// Toss a coin and insert new links until a loss
	while (rand_int(0, 1)) {
		Skip_link* last_link = ll->right();
		Skip_link* new_link = new Skip_link(sl->get_value());
		new_link->set_index(new_index);
		new_link->set_layer(ll->get_layer() + 1);
		new_link->set_down(last_link);
		last_link->set_up(new_link);

		if (ul) { // Set links with the upper limit for the layer if it exists
			while (!ul->up() && ul) {
				ul = ul->right();
			}
			if (ul) {
				ul = ul->up();
				ul->set_left(new_link);
				new_link->set_right(ul);
			}
		}

		while (!ll->up()) { //Set links with the lower limit for the layer
			ll = ll->left();
		}
		ll = ll->up();
		ll->set_right(new_link);
		new_link->set_left(ll);

		if (new_link->get_layer() == top_left->get_layer()) { //Increment the head layer if equalled by the new link.
			Skip_link* head_link = new Skip_link(this->get_value());
			head_link->set_layer(top_left->get_layer() + 1);
			head_link->set_down(top_left);
			top_left->set_up(head_link);
			top_left = head_link;
		}

	}

	return this;
}

Skip_link* Skip_link::remove(Skip_link* sl) { //Removes link and any other links with the same value then return the head.

	if (sl == nullptr) {
		cerr << "Tried to remove a link using a null pointer!\n";
		return this;
	}

	int position = sl->get_index();
	Skip_link* cursor = this;
	Skip_link* cursor_right = this->right();

	while (position) { //Position the cursor
		cursor = cursor->right();
		--position;
		if (!cursor) {
			error("Skip links position was outside the possible range and so wasn't removed");
			return this;
		}
	}

	if (cursor->get_value() != sl->get_value()) { //Check values allign
		error("Tried to remove a link which isn't in the skip list!");
		return this;
	}

	// Decrement indicies
	Skip_link* right_of_deletion = cursor->right();
	while (right_of_deletion) {

		Skip_link* d_cursor = right_of_deletion;

		while (d_cursor) {
			d_cursor->decrement_index();
			d_cursor = d_cursor->right();
		}
		while (!right_of_deletion->up()) {
			if (!right_of_deletion->right()) {
				break;
			}
			right_of_deletion = right_of_deletion->right();
		}
		right_of_deletion = right_of_deletion->up();
	}

	while (cursor) {


		Skip_link* cursor_r = cursor->right();
		Skip_link* cursor_l = cursor->left();

		if (cursor_r) { cursor_r->set_left(cursor_l); }
		if (cursor_l) { cursor_l->set_right(cursor_r); }

		cursor = cursor->up();

		if (!sl->get_index() && cursor && !cursor_r->up()) { //Add in new head positions when head is removed
			Skip_link* new_head_link = new Skip_link{ cursor_r->get_value() };
			new_head_link->set_layer(cursor_r->get_layer() + 1);
			new_head_link->set_down(cursor_r);
			cursor_r->set_up(new_head_link);
			new_head_link->set_right(cursor->right());
			cursor->set_right(new_head_link);
		}
	}

	if (sl->get_index()) { //Remove any extra head links which result from the remove
		Skip_link* top_link = get_top_link(this);
		while (top_link->down() && !top_link->right()) {

			if (!top_link->down()->right()) {
				top_link->down()->set_up(nullptr);
			}


			top_link = top_link->down();
		}
	}
	else {
		Skip_link* top_link = get_top_link(cursor_right);
		while (top_link->down() && !top_link->right()) {

			if (!top_link->down()->right()) {
				top_link->down()->set_up(nullptr);
			}

			top_link = top_link->down();
		}

	}

	return (sl->get_index()) ? this : cursor_right; //Return this unless this was removed in which case cursor right is the new base.
}

Skip_link* Skip_link::find(string s) { //Returns skip link with corresponding string on nullptr if not found.
	Skip_link* ll = this; //lower limit

	while (ll->up()) { // set ll to the highest layer
		ll = ll->up();
	}

	if (s == ll->get_value()) {
		return ll;
	}

	if (s < ll->get_value()) {
		return nullptr;
	}

	while (ll) {

		while (ll->right() && ll->right()->get_value() < s) {
			ll = ll->right();
		}

		if (ll->right() && ll->right()->get_value() == s) {
			return ll->right();
		}

		ll = ll->down();

	}
	return nullptr;
}

Skip_link* Skip_link::get_link(int index) { //Returns the skip link at the corresponding index. Otherwise returns nullpointer if out of range
	if (index < 0) {
		return nullptr;
	}
	Skip_link* cursor = this;
	while (index) {
		if (cursor) {
			cursor = cursor->right();
			--index;
		}
		else {
			return nullptr;
		}
	}
	return cursor;
}

void print_skip_list(Skip_link* skip_list) {
	Skip_link* cursor = skip_list;
	Skip_link* column = skip_list;

	while (column) {
		while (cursor) {
			cout << cursor->get_value() << ' ';
			cursor = cursor->right();
		}
		cout << '\n';
		column = column->up();
		cursor = column;
	}
}

void do_stuff() {
	Skip_link* tetlo = new Skip_link("tetlo");
	Skip_link* ghi = new Skip_link("ghi");

	Skip_link* test = new Skip_link("test 1");
	test = test->insert(tetlo)->insert(new Skip_link("howdy"))->insert(new Skip_link("hi"))
		->insert(new Skip_link("howdydoo"))->insert(ghi)->insert(new Skip_link("ghii"));
	//test = test->remove(test->find("ghi"))->remove(tetlo)->remove(test->find("ghii"));
	//test = test->remove(test->find("ghi"))->remove(tetlo);
	//test = test->remove(test->find("ghi"));
	//Skip_link* tetlo_found = test->find("tetlo");
	//Skip_link* tetlo_found_again = test->get_link(10); //Appears to function

	print_skip_list(test);
	//print_skip_list(tetlo_found);
	//print_skip_list(tetlo_found_again); //Doesn't appear to be printing layers, are they actually being added properly?

	//delete test;

	vector<Skip_link*> destroy;
	Skip_link* cursor = test;
	Skip_link* column = test;
	while (column) {
		while (cursor) {
			Skip_link* cursor_to_delete = cursor;
			cursor = cursor->right();
			destroy.push_back(cursor_to_delete);
		}
		column = column->up();
		cursor = column;
	}

	for (Skip_link* sl : destroy) {
		delete sl;
	}

	//cout << "Constructed: " << constructed << '\n';
	//cout << "Destructed: " << destructed << '\n';

}

int main() {
	try
	{
		do_stuff();



		cout << "Constructed: " << constructed << '\n';
		cout << "Destructed: " << destructed << '\n';

	}
	catch (exception& e)
	{
		cout << "Exception: " << e.what();
		return 1;
	}
	catch (...) {
		cout << "Big ol' error";
		return 2;
	}
}


