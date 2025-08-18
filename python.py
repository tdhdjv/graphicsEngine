import json
import base64

def main() -> None:
    with open("cube.gltf") as file:
        gltf = json.load(file)
    meshes = gltf['meshes']
    assert(len(meshes) == 1)
    primitives = meshes[0]['primitives']
    assert(len(primitives) == 1)
    attributes = primitives[0]['attributes']
    position_attributes_index =  attributes["POSITION"]

    pos_attribute = gltf["accessors"][position_attributes_index]
    assert pos_attribute['componentType'] == 5126
    assert pos_attribute['type'] == 'VEC3'

    position_buff_view_index = pos_attribute['bufferView']
    buff_view = gltf['bufferViews'][position_buff_view_index]

    #offs = buffer_view['byteOffset']
    #length = buffer_view['byteLength']
    buff_ind = buff_view["buffer"]

    uri = gltf["buffers"][buff_ind]["uri"]
    base_data = str.split(uri, ';')
    if(len(base_data) == 2):
        base, data = str.split(base_data[1], ",");
        if(base == "base64"):
            print(base64.b64decode(data))
        if(base == "base32"):
            print(base64.b32decode(data))
        if(base == "base16"):
            print(base64.b16decode(data))
    else:
        print(uri)


if __name__ == "__main__":
    main()
