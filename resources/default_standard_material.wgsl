#implements StandardMaterial;

struct MaterialProperties{
    BaseColorFactor: vec3f,
    Hardness: f32,
    Kd: f32,
    Ks: f32,
    NormalStrength: f32,
}

fn vertex(){
    WORLD_POSITION += vec3f(0, 0, sin(TIME));
}

fn fragment(){
    ALBEDO = vec3f(1, 0, 1);
}

fn light(){

}