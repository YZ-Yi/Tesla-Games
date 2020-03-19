#include "Menu.h"

void Menu::loadVertices() {
	HUD* background = new HUD();
	background->createHUD(backgroundVertices, indices, 20, 6);
	HUDList.push_back(background);

	HUD* trackImg = new HUD();
	trackImg->createHUD(trackImgVertices, indices, 20, 6);
	HUDList.push_back(trackImg);

	HUD* trackName = new HUD();
	trackName->createHUD(trackNameVertices, indices, 20, 6);
	HUDList.push_back(trackName);

	HUD* left1 = new HUD();
	left1->createHUD(left1Vertices, indices, 20, 6);
	HUDList.push_back(left1);

	HUD* right1 = new HUD();
	right1->createHUD(right1Vertices, indices, 20, 6);
	HUDList.push_back(right1);

	HUD* ai = new HUD();
	ai->createHUD(aiVertices, indices, 20, 6);
	HUDList.push_back(ai);

	HUD* aiNum = new HUD();
	aiNum->createHUD(aiNumVertices, indices, 20, 6);
	HUDList.push_back(aiNum);

	HUD* left2 = new HUD();
	left2->createHUD(left2Vertices, indices, 20, 6);
	HUDList.push_back(left2);

	HUD* right2 = new HUD();
	right2->createHUD(right2Vertices, indices, 20, 6);
	HUDList.push_back(right2);

	HUD* start = new HUD();
	start->createHUD(startVertices, indices, 20, 6);
	HUDList.push_back(start);

}

void Menu::loadTextures() {
	backgroundTexture = Texture("Textures/background.png");
	backgroundTexture.LoadTextureAlpha();

	dig0Texture = Texture("Textures/numbers/0.png");
	dig0Texture.LoadTextureAlpha();
	dig1Texture = Texture("Textures/numbers/1.png");
	dig1Texture.LoadTextureAlpha();
	dig2Texture = Texture("Textures/numbers/2.png");
	dig2Texture.LoadTextureAlpha();
	dig3Texture = Texture("Textures/numbers/3.png");
	dig3Texture.LoadTextureAlpha();
	dig4Texture = Texture("Textures/numbers/4.png");
	dig4Texture.LoadTextureAlpha();
	dig5Texture = Texture("Textures/numbers/5.png");
	dig5Texture.LoadTextureAlpha();
	dig6Texture = Texture("Textures/numbers/6.png");
	dig6Texture.LoadTextureAlpha();
	dig7Texture = Texture("Textures/numbers/7.png");
	dig7Texture.LoadTextureAlpha();
	dig8Texture = Texture("Textures/numbers/8.png");
	dig8Texture.LoadTextureAlpha();
	dig9Texture = Texture("Textures/numbers/9.png");
	dig9Texture.LoadTextureAlpha();

	track1Texture = Texture("Textures/track1.png");
	track1Texture.LoadTextureAlpha();
	track2Texture = Texture("Textures/track2.png");
	track2Texture.LoadTextureAlpha();
	right1Texture = Texture("Textures/right.png");
	right1Texture.LoadTextureAlpha();
	right2Texture = Texture("Textures/right2.png");
	right2Texture.LoadTextureAlpha();
	left1Texture = Texture("Textures/left.png");
	left1Texture.LoadTextureAlpha();
	left2Texture = Texture("Textures/left2.png");
	left2Texture.LoadTextureAlpha();
	track11Txt = Texture("Textures/t11.png");
	track11Txt.LoadTextureAlpha();
	track12Txt = Texture("Textures/t12.png");
	track12Txt.LoadTextureAlpha();
	track21Txt = Texture("Textures/t21.png");
	track21Txt.LoadTextureAlpha();
	track22Txt = Texture("Textures/t22.png");
	track22Txt.LoadTextureAlpha();
	ai1Txt = Texture("Textures/ai1.png");
	ai1Txt.LoadTextureAlpha();
	ai2TXt = Texture("Textures/ai2.png");
	ai2TXt.LoadTextureAlpha();
	start1Txt = Texture("Textures/start.png");
	start1Txt.LoadTextureAlpha();
	start2Txt = Texture("Textures/start2.png");
	start2Txt.LoadTextureAlpha();
	
	trackTexture = track1Texture;
	leftdownTexture = left1Texture;
	leftupTexture = left1Texture;
	rightdownTexture = right1Texture;
	rightupTexture = right1Texture;
	trackTxt = track12Txt;
	aiNumTxt = dig1Texture;
	aiTxt = ai1Txt;
	startTxt = start1Txt;

}

void Menu::loadShader() {
	hudShader.createHUDFromFiles(vHshader, fHshader);
}

void Menu::load() {
	loadShader();
	loadTextures();
	loadVertices();
}

void Menu::use() {
	hudShader.UseShader();

	GLint uniformModel = hudShader.GetModelLocation();
	GLint uniformProjection = hudShader.GetProjectionLocation();

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 ortho = glm::ortho(0.0f, (float)mainWindow.getWidth(), (float)mainWindow.getHeight(), 0.0f);						//orthograohic projection

	glDisable(GL_DEPTH_TEST);																									//disable the depth-testing

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(ortho));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	backgroundTexture.UseTexture();
	HUDList[0]->renderHUD();

	trackTexture.UseTexture();
	HUDList[1]->renderHUD();

	trackTxt.UseTexture();
	HUDList[2]->renderHUD();

	leftupTexture.UseTexture();
	HUDList[3]->renderHUD();

	rightupTexture.UseTexture();
	HUDList[4]->renderHUD();

	aiTxt.UseTexture();
	HUDList[5]->renderHUD();

	aiNumTxt.UseTexture();
	HUDList[6]->renderHUD();

	leftdownTexture.UseTexture();
	HUDList[7]->renderHUD();

	rightdownTexture.UseTexture();
	HUDList[8]->renderHUD();

	startTxt.UseTexture();
	HUDList[9]->renderHUD();
	glEnable(GL_DEPTH_TEST);
}
