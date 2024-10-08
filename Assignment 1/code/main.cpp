/**
@file main.cpp
*/

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "Image.h"
#include "Material.h"

using namespace std;

/**
 Class representing a single ray.
 */
class Ray{
public:
    glm::vec3 origin; ///< Origin of the ray
    glm::vec3 direction; ///< Direction of the ray
	/**
	 Constructor of the ray
	 @param origin Origin of the ray
	 @param direction Direction of the ray
	 */
    Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction){
    }
};


class Object;

/**
 Structure representing the even of hitting an object
 */
struct Hit{
    bool hit; ///< Boolean indicating whether there was or there was no intersection with an object
    glm::vec3 normal; ///< Normal vector of the intersected object at the intersection point
    glm::vec3 intersection; ///< Point of Intersection
    float distance; ///< Distance from the origin of the ray to the intersection point
    Object *object; ///< A pointer to the intersected object
};

/**
 General class for the object
 */
class Object{
	
public:
	glm::vec3 color; ///< Color of the object
	Material material; ///< Structure describing the material of the object
	/** A function computing an intersection, which returns the structure Hit */
    virtual Hit intersect(Ray ray) = 0;

	/** Function that returns the material struct of the object*/
	Material getMaterial(){
		return material;
	}
	/** Function that set the material
	 @param material A structure describing the material of the object
	*/
	void setMaterial(Material material){
		this->material = material;
	}
};

/**
 Implementation of the class Object for sphere shape.
 */
class Sphere : public Object{
private:
    float radius; ///< Radius of the sphere
    glm::vec3 center; ///< Center of the sphere

public:
	/**
	 The constructor of the sphere
	 @param radius Radius of the sphere
	 @param center Center of the sphere
	 @param color Color of the sphere
	 */
    Sphere(float radius, glm::vec3 center, glm::vec3 color) : radius(radius), center(center){
		this->color = color;
    }
	Sphere(float radius, glm::vec3 center, Material material) : radius(radius), center(center){
		this->material = material;
	}
	/** Implementation of the intersection function*/
	Hit intersect(Ray ray){
		
		Hit hit;
		hit.hit = false;
		
		/* ------------------ Exercise 1 -------------------
		 
		Place for your code: ray-sphere intersection. Remember to set all the fields of the hit structure:

		hit.intersection =
		hit.normal =
		hit.distance =
		hit.object = this;
		 
		------------------------------------------------- */

    	/**
    	Translation of coordinate system
    	Old coordinate system (O) has origin at (x0, y0).
    	New coordinate system (N) has origin at (x1, y1).
    	A coordinate (x, y), in the new system (x', y') is:
    	x' = x - (x1 - x0)
    	y' = y - (y1 - y0)
    	*/

        // TODO: add f to floats

    	// Translate variables to new coordinate system
		glm::vec3 translationOffset = ray.origin;
    	ray.origin -= translationOffset;
    	center -= translationOffset;

    	// Variable names are taken from notes
		float a = glm::dot(center, ray.direction);
    	float d = std::sqrt(
			std::pow(glm::length(center), 2) -
			std::pow(a, 2)
		);
    	float closest_t;
		if (d == radius) {
			closest_t = a;
		} else if (d < radius) {
			float b = std::sqrt(
				std::pow(radius, 2) -
				std::pow(d, 2)
			);
			float t1 = a - b;
			float t2 = a + b;

			// check if camera is not inside sphere
			if (t1 < 0 && 0 < t2) {
				closest_t = INFINITY;
			} else {
				t1 = t1 >= 0 ? t1 : INFINITY;
				t2 = t2 >= 0 ? t2 : INFINITY;
				closest_t = std::min(t1, t2);
			}
		} else {
			closest_t = INFINITY;
		}

		// Translate back object variables to old coordinate system
		ray.origin += translationOffset;
		center += translationOffset;

		if (closest_t != INFINITY) {
			hit.hit = true;
			hit.intersection = ray.direction * closest_t;

			// Translate back intersection to old coordinate system
			hit.intersection += ray.origin;

			hit.distance = glm::distance(ray.origin, hit.intersection);
			hit.normal = glm::normalize(hit.intersection - center);
			hit.object = this;
		}
		return hit;
	}
};

/**
 Light class
 */
class Light{
public:
	glm::vec3 position; ///< Position of the light source
	glm::vec3 color; ///< Color/intensity of the light source
	Light(glm::vec3 position): position(position){
		color = glm::vec3(1.0);
	}
	Light(glm::vec3 position, glm::vec3 color): position(position), color(color){
	}
};

vector<Light *> lights; ///< A list of lights in the scene
glm::vec3 ambient_light(1.0);
vector<Object *> objects; ///< A list of all objects in the scene


/** Function for computing color of an object according to the Phong Model
 @param point A point belonging to the object for which the color is computer
 @param normal A normal vector the the point
 @param view_direction A normalized direction from the point to the viewer/camera
 @param material A material structure representing the material of the object
*/
glm::vec3 PhongModel(glm::vec3 point, glm::vec3 normal, glm::vec3 view_direction, Material material){

	glm::vec3 color(0.0);
	
	/* ------------------Exercise 3--------------------
	 
	 Phong model.
	 Your code should implement a loop over all the lighthouses in the array lights and aggregate the contribution of each of them to the final color of the object.
	 Outside the loop add also the ambient component from ambient_light.
				 
	 ------------------------------------------------- */

    // self-emitting intensity + ambient
    color += material.ambient * ambient_light;

    for (Light* light : lights) {
        // diffuse
        glm::vec3 light_direction = glm::normalize(light->position - point);
        float cosOmega = glm::dot(normal, light_direction);
        if (cosOmega > 0) {
            color += material.diffuse * cosOmega * light->color;
        }

        // specular
        glm::vec3 reflex_direction = glm::normalize(2.0f * normal * glm::dot(normal, light_direction) - light_direction);
        float cosAlpha = glm::dot(view_direction, reflex_direction);
        if (cosAlpha > 0) {
            color += material.specular * glm::pow(cosAlpha, material.shininess) * light->color;
        }
    }
	
	// The final color has to be clamped so the values do not go beyond 0 and 1.
	color = glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
	return color;
}

/**
 Functions that computes a color along the ray
 @param ray Ray that should be traced through the scene
 @return Color at the intersection point
 */
glm::vec3 trace_ray(Ray ray){

	Hit closest_hit;

	closest_hit.hit = false;
	closest_hit.distance = INFINITY;

	for(int k = 0; k<objects.size(); k++){
		Hit hit = objects[k]->intersect(ray);
		if(hit.hit == true && hit.distance < closest_hit.distance)
			closest_hit = hit;
	}

	glm::vec3 color(0.0);

	if(closest_hit.hit){
		/* ------------------Exercise 3--------------------
		 
		 Use the second line when you implement PhongModel function - Exercise 3
					 
		 ------------------------------------------------- */
		// color = closest_hit.object->color;
        color = PhongModel(closest_hit.intersection, closest_hit.normal, glm::normalize(-ray.direction), closest_hit.object->getMaterial());
	}else{
		color = glm::vec3(0.0, 0.0, 0.0);
	}
	return color;
}
/**
 Function defining the scene
 */
void sceneDefinition (){

	// objects.push_back(new Sphere(1.0, glm::vec3(0,-2,8), glm::vec3(0.6, 0.9, 0.6)));

	/* ------------------Exercise 2--------------------
	 
	Place for your code: additional sphere
	 
	------------------------------------------------- */

    //	objects.push_back(new Sphere(1.0, glm::vec3(1,-2,8), glm::vec3(0.6, 0.6, 0.9)));
	
	
	/* ------------------Exercise 3--------------------
	 
	 Add here all the objects to the scene. Remember to add them using the constructor for the sphere with material structure.
	 You will also need to define the materials.
	 Example of adding one sphere:
	 
	 Material red_specular;
	 red_specular.diffuse = glm::vec3(1.0f, 0.3f, 0.3f);
	 red_specular.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
	 red_specular.specular = glm::vec3(0.5);
	 red_specular.shininess = 10.0;
	 
	 objects.push_back(new Sphere(0.5, glm::vec3(-1,-2.5,6), red_specular));
	 
	 Remember also about adding some lights. For example a white light of intensity 0.4 and position in (0,26,5):
	 
	 lights.push_back(new Light(glm::vec3(0, 26, 5), glm::vec3(0.4)));
	 
	------------------------------------------------- */

    Material blueMaterial;
    blueMaterial.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    blueMaterial.diffuse = glm::vec3(0.7f, 0.7f, 1.0f);
    blueMaterial.specular = glm::vec3(0.6f, 0.6f, 0.6f);
    blueMaterial.shininess = 100.0f;

    Material redMaterial;
    redMaterial.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
    redMaterial.diffuse = glm::vec3(1.0f, 0.3f, 0.3f);
    redMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    redMaterial.shininess = 10.0f;

    Material greenMaterial;
    greenMaterial.ambient = glm::vec3(0.07f, 0.09f, 0.07f);
    greenMaterial.diffuse = glm::vec3(0.7f, 0.9f, 0.7f);
    greenMaterial.specular = glm::vec3(0.0f, 0.0f, 0.0f);
    greenMaterial.shininess = 0.0f;

    objects.push_back(new Sphere(1.0f,
                                 glm::vec3(1.0f, -2.0f, 8.0f),
                                 blueMaterial));
    objects.push_back(new Sphere(0.5f,
                                 glm::vec3(-1.0f, -2.5f, 6.0f),
                                 redMaterial));
    objects.push_back(new Sphere(1.0f,
                                 glm::vec3(2.0f, -2.0f, 6.0f),
                                 greenMaterial));

    lights.push_back(new Light(glm::vec3(0.0f, 26.0f, 5.0f), glm::vec3(0.4)));
    lights.push_back(new Light(glm::vec3(0.0f, 1.0f, 12.0f), glm::vec3(0.4)));
    lights.push_back(new Light(glm::vec3(0.0f, 5.0f, 1.0f), glm::vec3(0.4)));

}

int main(int argc, const char * argv[]) {

    clock_t t = clock(); // variable for keeping the time of the rendering

    int width = 1024; //width of the image
    int height = 768; // height of the image
    float fov = 90; // field of view

	sceneDefinition(); // Let's define a scene

	Image image(width,height); // Create an image where we will store the result

	/* ------------------ Exercise 1 -------------------
	 
	Place for your code: Loop over pixels to form and traverse the rays through the scene
	 
	------------------------------------------------- */
    const float PIXEL_SIZE = (2 * std::tan((fov * M_PI / 180) / 2)) / width;
    // (X, Y) = top left corner coordinate
	const float X = -(width * PIXEL_SIZE) / 2;
	const float Y = (height * PIXEL_SIZE) / 2;
	glm::vec3 origin(0, 0, 0);
	float z = 1.0f;
    for(int w = 0; w < width; w++)
        for(int h = 0; h < height; h++){

			/* ------------------ Exercise 1 -------------------
			 
			Place for your code: ray definition for pixel (i,j), ray traversal

        	Definition of the ray
			glm::vec3 origin(0, 0, 0);
			glm::vec3 direction(?, ?, ?);               // fill in the correct values
			direction = glm::normalize(direction);

			Ray ray(origin, direction);  // ray traversal

			image.setPixel(i, j, trace_ray(ray));

			------------------------------------------------- */

			float x = X + (w + 0.5f) * PIXEL_SIZE;
        	float y = Y - (h + 0.5f) * PIXEL_SIZE;
        	glm::vec3 direction = glm::normalize(glm::vec3(x, y, z));
        	Ray ray = Ray(origin, direction);
        	image.setPixel(w, h, trace_ray(ray));

        }

    t = clock() - t;
    cout<<"It took " << ((float)t)/CLOCKS_PER_SEC<< " seconds to render the image."<< endl;
    cout<<"I could render at "<< (float)CLOCKS_PER_SEC/((float)t) << " frames per second."<<endl;

	// Writing the final results of the rendering
	if (argc == 2){
		image.writeImage(argv[1]);
	}else{
		image.writeImage("./result.ppm");
	}

	
    return 0;
}
