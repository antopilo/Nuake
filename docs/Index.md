# Nuake engine





# UI system
The UI system uses yoga layout and custom styling properties. The styling properties will mimick the standard css properties but will be rendererd using a shader. Since the layout system is based on yoga, the actual writting of the layout will look a lot like html, the differences will lie in the actual components.

# The UI pipeline maybe

1. load xml file containing pseudo html with components
  Potential nodes
- Canvas basically a container for UI.
- Button with basic state and event
- Texture which display a engine texture(can use framebuffer!!)
- Slider(ouch)

2. Create an in engien structure of the layout

3. Create a yoga layotu from node and calculate when changes occurs

4. Draw each node in the structure using styling loaded somewhere idk.


# Styling
TODO MAN

