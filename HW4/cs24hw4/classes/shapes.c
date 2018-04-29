#include "shapes.h"
#include <assert.h>
#include <stdlib.h>


/* ===========================================================================
 * Global State
 *
 * These are the instances of class - information used by objects of the 
 * various types.  Pass a pointer to these to the various object-
 * init functions.
 */

static Shape_Class  Shape;
static Box_Class    Box;
static Sphere_Class Sphere;
static Cone_Class   Cone;


/* ===========================================================================
 * General Functions
 */


/* !
 * This function performs static initialization of all classes.  It must be
 * called before any of the classes can be used.
 */  
void static_init() {
    Shape_class_init(&Shape);
    Box_class_init(&Box);
    Sphere_class_init(&Sphere);
    Cone_class_init(&Cone);
}


/* ===========================================================================
 * Shape base class
 */


/* ! Static initialization for the Shape class. */
void Shape_class_init(Shape_Class *class) {
  /* Allocate memory for the getVolume function pointer, and set it to
     NULL as there is no implementation */
  class->getVolume = malloc(sizeof(float));
  class->getVolume = NULL;
}


/* !
 * Object initialization (i.e. the constructor) for the Shape class.  This
 * function initializes the density of the shape, as well as the class info.
 */
void Shape_init(Shape_Data *this, Shape_Class *class, float D) {
  /* Set the class of the object */
  this->class = class;
  /* Set the density of the object */
  Shape_setDensity(this, D);
}


/* ! Sets the density of this shape.  The argument must be nonnegative! */
void Shape_setDensity(Shape_Data *this, float D) {
  /* Check that the density is non-negative */
  assert(D >= 0);
  this->density = D;
}


/* ! Returns the mass of this shape, computed from the density and volume. */
float Shape_getMass(Shape_Data *this) {
  /* The mass is density times volume */
  return this->density * this->class->getVolume(this);
}


/*
 * THERE IS NO Shape_getVolume() FUNCTION, because Shape doesn't provide an
 * implementation!  In the class initialization, set the function - pointer to
 * NULL to signify this.
 */
 
/*
 * There is also no new_Shape() function, since Shape is abstract (missing
 * some of its implementation), and therefore is not instantiable.
 */


/* ===========================================================================
 * Box subclass
 */


/* ! Static initialization for the Box class. */
void Box_class_init(Box_Class *class) {
  /* Set the getVolume function pointer to the box implementation */
  class->getVolume = Box_getVolume;
}


/* !
 * Object initialization (i.e. the constructor) for the Box class.  This
 * function first calls the Shape constructor to initialize the class info and
 * density, and then it initializes its data members with the specified 
 * values.
 */
void Box_init(Box_Data *this, Box_Class *class,
    float L, float W, float H, float D) {
  /* Initialize the Box object using the superconstructor */
  Shape_init((Shape_Data *)this, (Shape_Class *)class, D);
  /* Initialize the box class */
  Box_class_init(class);
  /* Set the size of the box */
  Box_setSize(this, L, W, H);
}


/* !
 * This function implements the operation corresponding to the C++ code
 * "new Box(L, W, H, D)", performing both heap - allocation and 
 * initialization. 
 */
Box_Data * new_Box(float L, float W, float H, float D) {
  /* Allocate memory for the box data */
  Box_Data *this = malloc(sizeof(Box_Data));
  /* Initialize the box object and return a reference to it */
  Box_init(this, &Box, L, W, H, D);
  return this;
}


/* !
 * Sets the dimensions of the box.  The arguments are asserted to be positive.
 */
void Box_setSize(Box_Data *this, float L, float W, float H) {
  /* Assert that length, width, and height are positive. */
  assert(L > 0);
  assert(W > 0);
  assert(H > 0);
  this->length = L;
  this->width = W;
  this->height = H;
}


/* !
 * Computes the volume of the box.  This function provides the implementation
 * of Shape::getVolume(), which is abstract (i.e. pure - virtual).
 */
float Box_getVolume(Box_Data *this) {
  /* The volume of a box is the length times width times height */
  return this->length * this->width * this->height;
}


/* ===========================================================================
 * Sphere subclass
 */


/* ! Static initialization for the Sphere class. */
void Sphere_class_init(Sphere_Class *class) {
  /* Set the getVolume function pointer to the sphere's function */
  class->getVolume = Sphere_getVolume;
}


/* !
 * Object initialization (i.e. the constructor) for the Sphere class.  This
 * function first calls the Shape constructor to initialize the class info and
 * density, and then it initializes its data members with the specified 
 * values.
 */
void Sphere_init(Sphere_Data *this, Sphere_Class *class, float R, float D) {
  /* Initialize the sphere object using the superconstructor */
  Shape_init((Shape_Data *)this, (Shape_Class *)class, D);
  /* Initialize the sphere class */
  Sphere_class_init(class);
  /* Set the radius of the sphere */
  Sphere_setRadius(this, R);
}


/* !
 * This function implements the operation corresponding to the C++ code
 * "new Sphere(R, D)", performing both heap - allocation and initialization. 
 */
Sphere_Data * new_Sphere(float R, float D) {
  /* Allocate memory for the sphere data */
  Sphere_Data *this = malloc(sizeof(Sphere_Data));
  /* Initialize the sphere object */
  Sphere_init(this, &Sphere, R, D);
  return this;
}


/* ! Sets the radius of the sphere.  The argument is asserted to be 
 * positive. */
void Sphere_setRadius(Sphere_Data *this, float R) {
  /* Assert that the radius is positive */
  assert(R > 0);
  this->radius = R;
}


/* !
 * Computes the volume of the sphere.  This function provides the 
 * implementation of Shape::getVolume(), which is abstract 
 * (i.e. pure - virtual).
 */
float Sphere_getVolume(Sphere_Data *this) {
  /* The volume of a sphere is 4/3 pi r^3 */
  return 4.0 / 3.0 * 3.14159 * this->radius * this->radius * this->radius;
}


/* ===========================================================================
 * Cone subclass
 */


/* ! Static initialization for the Cone class. */
void Cone_class_init(Cone_Class *class) {
  /* Set the function pointer getVolume to the cone's volume function */
  class->getVolume = Cone_getVolume;
}


/* !
 * Object initialization (i.e. the constructor) for the Cone class.  This
 * function first calls the Shape constructor to initialize the class info and
 * density, and then it initializes its data members with the specified 
 * values.
 */
void Cone_init(Cone_Data *this, Cone_Class *class, float BR,
               float H, float D) {
  /* Initialize the cone using the superconstructor */
  Shape_init((Shape_Data *)this, (Shape_Class *)class, D);
  /* Initialize the cone class */
  Cone_class_init(class);
  /* Set the base radius and height of the cone */
  Cone_setBaseHeight(this, BR, H);
}


/* !
 * This function implements the operation corresponding to the C++ code
 * "new Cone(BR, H, D)", performing both heap - allocation and 
 * initialization. 
 */
Cone_Data * new_Cone(float BR, float H, float D) {
  /* Allocate memory for the cone data */
  Cone_Data *this = malloc(sizeof(Cone_Data));
  /* Initialize the cone object */
  Cone_init(this, &Cone, BR, H, D);
  return this;
}


/* !
 * Sets the dimensions of the cone.  The arguments are asserted to 
 * be positive.
 */
void Cone_setBaseHeight(Cone_Data *this, float BR, float H) {
  /* Assert that base radius and height are positive */
  assert(BR > 0);
  assert(H > 0);
  this->base_radius = BR;
  this->height = H;
}


/* !
 * Computes the volume of the cone.  This function provides the implementation
 * of Shape::getVolume(), which is abstract (i.e. pure - virtual).
 */
float Cone_getVolume(Cone_Data *this) {
  /* The volume of a cone is 1/3 pi r^2 h */
  return (1.0 / 3.0) * 3.14159 * this->base_radius *
    this->base_radius * this->height;
}

