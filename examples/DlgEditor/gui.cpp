// ===================================================================================================================
// Widget Example
// Drawing standard ImGui widgets inside the node body
//
// First, some unsorted notes about which widgets do and don't draw well inside nodes.  Run the examples to see all the allowed widgets.
//
// - Child windows with scrolling doesn't work in the node.  The child window appears in a normal node,
//   and scrolls, but its contents are floating around in the wrong location, and they are not scaled.
//   Note that you can put scrolling child windows into "deferred popups" (see next item).
// - Listboxes and combo-boxes only work in nodes with a work-around: deferring the popup calls until after the node drawing is
//   completed. Look to the popup-demo for an example.
// - Headers and trees work inside the nodes only with hacks.  This is because they attempt to span the "avaialbe width"
//   and the nodes can't tell these widgets how wide it is. The work-around is to set up a fake
//   table with a static column width, then draw your header and tree widgets in that column.
// - Clickable tabs don't work in nodes.  Tabs appear, but you cannot actually change the tab, so they're functionally useless.
// - Editable text areas work, but you have to manually manage disabling the editor shorcuts while typing is detected.
//   Look around for the call to ed::EnableShortcuts() for an example.
// - Most of the cool graph widgets can't be used because they are hard-coded in ImGui to spawn tooltips, which don't work.

#include <gui.hpp>
#include <gui.h>
#include <stdio.h>

int renderBool(struct TreeNode *node, uint32_t flags)
{
    bool option = (bool)(*(node->data.staticBuffer) - 30);
    ImGui::Text("Checked: %s", option ? "true" : "false");
    ImGui::Checkbox("Option 1", &option);
    *(node->data.staticBuffer) = (uint8_t)(option) + 30;
    // printf("%d\n", option);
    return 0;
}

int renderFloat(struct TreeNode *node, uint32_t flags)
{
    float f0 = *(float *)(node->data.staticBuffer);
    ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");
    *(float *)(node->data.staticBuffer) = f0;
    return 0;
}

int renderInt4(struct TreeNode *node, uint32_t flags)
{
    int i0 = *(int *)(node->data.staticBuffer);
    ImGui::InputInt("input int", &i0);
    *(int *)(node->data.staticBuffer) = i0;
    return 0;
}

int renderSymbol(struct TreeNode *node, uint32_t flags)
{
    char textBuffer[19];
    sprintf(textBuffer, "0x%02X%02X%02X%02X%02X%02X%02X%02X", node->data.staticBuffer[7], node->data.staticBuffer[6], node->data.staticBuffer[5], node->data.staticBuffer[4], node->data.staticBuffer[3], node->data.staticBuffer[2], node->data.staticBuffer[1], node->data.staticBuffer[0]);
    ImGui::InputText("Hex", textBuffer, 19);
    sscanf(textBuffer + 2, "%" PRIx64, (uint64_t *)(node->data.staticBuffer));
    return 0;
}

int renderString(struct TreeNode *node, uint32_t flags)
{
    char stringBuffer[512];
    if (node->dataSize <= sizeof(node->data))
    {
        memcpy(stringBuffer, (char *)(node->data.staticBuffer + 4), node->dataSize - 4);
    }
    else
    {
        memcpy(stringBuffer, (char *)(node->data.dynamicBuffer + 4), node->dataSize - 4);
    }
    stringBuffer[node->dataSize - 4] = '\0';
    ImGui::InputText("String", stringBuffer, 512);
    size_t stringLength = strlen(stringBuffer);
    if (node->dataSize <= sizeof(node->data) && stringLength + 4 > sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        node->data.dynamicBuffer = (uint8_t *)malloc(node->dataSize);
        *(uint32_t *)node->data.dynamicBuffer = stringLength;
        memcpy(node->data.dynamicBuffer + 4, stringBuffer, stringLength);
    }
    else if (node->dataSize > sizeof(node->data) && stringLength + 4 <= sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        free(node->data.dynamicBuffer);
        *(uint32_t *)node->data.staticBuffer = stringLength;
        memcpy(node->data.staticBuffer + 4, stringBuffer, stringLength);
    }
    else if (node->dataSize > sizeof(node->data) && stringLength + 4 > sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        free(node->data.dynamicBuffer);
        node->data.dynamicBuffer = (uint8_t *)malloc(stringLength + 4);
        *(uint32_t *)node->data.dynamicBuffer = stringLength;
        memcpy(node->data.dynamicBuffer + 4, stringBuffer, stringLength);
    }
    else if (node->dataSize <= sizeof(node->data) && stringLength + 4 <= sizeof(node->data))
    {
        node->dataSize = stringLength + 4;
        *(uint32_t *)node->data.staticBuffer = stringLength;
        memcpy(node->data.staticBuffer + 4, stringBuffer, stringLength);
    }
    return 0;
    // return ImGui::InputText("String", (char *)(node->data.dynamicBuffer + 4), *(uint32_t *)(node->data.dynamicBuffer));
}

int renderVector3(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputFloat3("xyz", (float *)(node->data.dynamicBuffer));
}

int renderColor(struct TreeNode *node, uint32_t flags)
{
    return ImGui::InputFloat4("rgba", (float *)(node->data.dynamicBuffer));
}

int renderNode(struct TreeNode *node, int uniqueID, uint32_t flags)
{
    // printf("%d\n", node->childCount);
    if (node->childCount == 0)
    {
        renderMetaClass(node, flags);
        // printf("%d\n", option);

        return 0;
    }
    for (uint16_t i = 0; i < node->childCount; ++i)
    {
        char text[128];
        sprintf(text, "%s##%d-%d", getMetaClassName(node->children[i]->typeSymbol), uniqueID, i);
        if (ImGui::TreeNode(text))
        {
            renderNode(node->children[i], uniqueID, flags);
            ImGui::TreePop();
        }
    }
    return 0;
}

DlgApplication::DlgApplication(const char *name, FILE *inputStream) : Application(name)
{
    initializeMetaClassDescriptions();
    dlg.dataSize = 0;

    readMetaStreamHeader(inputStream, &(this->header));
    DlgRead(inputStream, &(this->dlg), 0);
    fclose(inputStream);

    CreateLinks();
}

void DlgApplication::CreateLinks()
{
    m_Links.reserve(1000);
    m_Nodes.reserve(2000);
    /* Iteration through all children in each folder */
    for (uint32_t i = 1; i < *(uint64_t *)(dlg.children[13]->children[0]->data.staticBuffer); ++i)
    {
        for (uint32_t j = 1; j < *(uint64_t *)(dlg.children[13]->children[i]->children[2]->children[0]->data.staticBuffer); ++j)
        {
            TreeNode *child = dlg.children[13]->children[i]->children[2]->children[j];
            AddNextLinks(child);
        }
    }

    for (uint32_t i = 1; i < *(uint64_t *)(dlg.children[14]->children[0]->data.staticBuffer); ++i)
    {
        TreeNode *node = dlg.children[14]->children[i];
        if (node->typeSymbol == 0x13998d6f0bde3491) // DlgNodeChoices
        {
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[1]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                AddNextLinks(child);
            }
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[2]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                AddNextLinks(child);
            }
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[3]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                AddNextLinks(child);
            }
        }
        else if (node->typeSymbol == 0x789758cb1a8d6628 || node->typeSymbol == 0x36e367b48ad63274 || node->typeSymbol == 0x97ba9139ccc1cf26) // DlgNodeConditional || DlgNodeSequence || DlgNodeParallel
        {
            for (uint32_t j = 1; j < *(uint64_t *)(node->children[1]->children[0]->children[0]->data.staticBuffer); ++j)
            {
                TreeNode *child = node->children[1]->children[0]->children[j];
                AddNextLinks(child);
            }
        }
    }
}

void DlgApplication::AddNextLinks(struct TreeNode *node)
{
    NodePin currentNodePin = NodePin{node, NodePin::PinType::next};
    this->m_Nodes.push_back(currentNodePin);
    if (*DlgGetNextID(node) == 0x988d0903f713877b)
    {
        return;
    }
    for (uint32_t i = 1; i < *(uint64_t *)(dlg.children[14]->children[0]->data.staticBuffer); ++i)
    {
        TreeNode *nextNode = dlg.children[14]->children[i];
        if (*DlgGetNextID(node) == *DlgGetID(nextNode))
        {
            NodePin nextNodePin = NodePin{nextNode, NodePin::PinType::prev};
            this->m_Nodes.push_back(nextNodePin);
            this->m_Links.push_back(LinkInfo{&m_Nodes[m_Nodes.size() - 2], &m_Nodes[m_Nodes.size() - 1]});
            AddNextLinks(nextNode);
        }
    }
}

void DlgApplication::OnStart()
{
    ed::Config config;
    config.SettingsFile = "Widgets.json";
    m_Context = ed::CreateEditor(&config);

    printf("Read dlog %d, %d\n", m_Links.size(), m_Nodes.size());
}

void DlgApplication::OnStop()
{
    ed::DestroyEditor(m_Context);
}

void DlgApplication::OnFrame(float deltaTime)
{
    static bool firstframe = true; // Used to position the nodes on startup
    auto &io = ImGui::GetIO();

    // FPS Counter Ribbon
    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
    ImGui::Separator();

    // Node Editor Widget
    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    float nodeX = 0;
    float nodeY = 0;

    // printf("loop start\n");

    for (uint32_t i = 0; i < m_Nodes.size(); ++i)
    {
        float width = 450; // bad magic numbers. used to define width of tree widget
        TreeNode *node = m_Nodes[i].parentNode;
        ed::BeginNode((uint64_t)node);
        char *metaClassName = getMetaClassName(node->typeSymbol);
        ImGui::Text(metaClassName);
        if (m_Nodes[i].pinType == NodePin::PinType::prev)
        {
            ed::BeginPin((uint64_t)(&m_Nodes[i]), ed::PinKind::Input);
            ImGui::Text("-> prev");
            ed::EndPin();
            ++i;
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(width / 2 + width / 4, 0)); //  magic number - Crude & simple way to nudge over the output pin. Consider using layout and springs
            ImGui::SameLine();
            ed::BeginPin((uint64_t)(&m_Nodes[i]), ed::PinKind::Output);
            ImGui::Text("next ->");
            ed::EndPin();
        }
        else
        {
            nodeY += 200;
            nodeX = 0;
            ImGui::Dummy(ImVec2(width / 2 + width / 4, 0)); //  magic number - Crude & simple way to nudge over the output pin. Consider using layout and springs
            ImGui::SameLine();
            ed::BeginPin((uint64_t)(&m_Nodes[i]), ed::PinKind::Output);
            ImGui::Text("next ->");
            ed::EndPin();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(width, 0));
        ImGui::PopStyleVar();

        // Start columns, but use only first one.
        char textBuffer[128];
        sprintf(textBuffer, "##%s%d", metaClassName, i);
        ImGui::BeginColumns(textBuffer, 2,
                            ImGuiOldColumnFlags_NoBorder |
                                ImGuiOldColumnFlags_NoResize |
                                ImGuiOldColumnFlags_NoPreserveWidths |
                                ImGuiOldColumnFlags_NoForceWithinWindow);

        // Adjust column width to match requested one.
        ImGui::SetColumnWidth(0, width + ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemSpacing.x);
        // End of tree column startup --------------------------------------------------------------

        // Back to normal ImGui drawing, in our column.

        if (ImGui::CollapsingHeader(textBuffer))
        {
            renderNode(node, i, 0);

            /*
            if (ImGui::TreeNode("Open Tree"))
            {
                static bool OP1_Bool = false;
                ImGui::Text("Checked: %s", OP1_Bool ? "true" : "false");
                ImGui::Checkbox("Option 1", &OP1_Bool);
                ImGui::TreePop();
                // printf("%d\n", OP1_Bool);
            }
            */
        }
        // Tree Column Shutdown
        ImGui::EndColumns();

        ed::EndNode(); // End of Tree Node Demo

        if (firstframe)
        {
            ed::SetNodePosition((uint64_t)node, ImVec2(nodeX, nodeY));
        }
        nodeX += 550;
    }

    // ==================================================================================================
    // Link Drawing Section

    for (auto &linkInfo : m_Links)
        ed::Link((uint64_t)&linkInfo, (uint64_t)(linkInfo.srcNode), (uint64_t)(linkInfo.destNode));

    // ==================================================================================================
    // Interaction Handling Section
    // This was coppied from BasicInteration.cpp. See that file for commented code.

    // Handle creation action ---------------------------------------------------------------------------
    if (ed::BeginCreate())
    {
        ed::PinId srcPinId, destPinId;
        if (ed::QueryNewLink(&srcPinId, &destPinId))
        {
            if (srcPinId && destPinId)
            {
                if (ed::AcceptNewItem())
                {
                    NodePin *srcPin = (NodePin *)(uint64_t)(srcPinId);
                    NodePin *destPin = (NodePin *)(uint64_t)(destPinId);
                    if (srcPin->pinType != destPin->pinType && !ed::PinHadAnyLinks(srcPinId) && !ed::PinHadAnyLinks(destPinId))
                    {
                        m_Links.push_back({srcPin, destPin});
                        ed::Link((uint64_t)(&(m_Links.back())), (uint64_t)m_Links.back().srcNode, (uint64_t)m_Links.back().destNode);
                        if (srcPin->pinType == NodePin::PinType::next)
                        {
                            *DlgGetNextID(srcPin->parentNode) = *DlgGetID(destPin->parentNode);
                            *DlgGetPrevID(destPin->parentNode) = *DlgGetID(srcPin->parentNode);
                        }
                        else
                        {
                            *DlgGetNextID(destPin->parentNode) = *DlgGetID(srcPin->parentNode);
                            *DlgGetPrevID(srcPin->parentNode) = *DlgGetID(destPin->parentNode);
                        }
                    }
                }
            }
        }
    }
    ed::EndCreate();

    // Handle deletion action ---------------------------------------------------------------------------
    if (ed::BeginDelete())
    {
        ed::LinkId deletedLinkId;
        /*
        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            if (ed::AcceptDeletedItem())
            {
                for (auto &link : m_Links)
                {
                    if (ax::NodeEditor::LinkId{} == deletedLinkId)
                    {
                        m_Links.erase(&link);
                        break;
                    }
                }
            }
        }
        */

        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            if (ed::AcceptDeletedItem())
            {
                LinkInfo *linkPtr = (LinkInfo *)(uint64_t)deletedLinkId;
                if (linkPtr->srcNode->pinType == NodePin::PinType::next)
                {
                    *DlgGetNextID(linkPtr->srcNode->parentNode) = 0x988d0903f713877b;
                    *DlgGetPrevID(linkPtr->destNode->parentNode) = 0x988d0903f713877b;
                }
                else
                {
                    *DlgGetNextID(linkPtr->destNode->parentNode) = 0x988d0903f713877b;
                    *DlgGetPrevID(linkPtr->srcNode->parentNode) = 0x988d0903f713877b;
                }

                m_Links.erase(linkPtr);
            }
        }
    }
    ed::EndDelete();

    ed::End();
    ed::SetCurrentEditor(nullptr);
    firstframe = false;
    // ImGui::ShowMetricsWindow();
    // ImGui::ShowDemoWindow();
}

struct Example : public Application
{
    using Application::Application;

    struct LinkInfo
    {
        ed::LinkId Id;
        ed::PinId InputId;
        ed::PinId OutputId;
    };

    void OnStart() override
    {
        ed::Config config;
        config.SettingsFile = "Widgets.json";
        m_Context = ed::CreateEditor(&config);
    }

    void OnStop() override
    {
        ed::DestroyEditor(m_Context);
    }

    void OnFrame(float deltaTime) override
    {
        static bool firstframe = true; // Used to position the nodes on startup
        auto &io = ImGui::GetIO();

        // FPS Counter Ribbon
        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
        ImGui::Separator();

        // Node Editor Widget
        ed::SetCurrentEditor(m_Context);
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));
        int uniqueId = 1;

        // Basic Widgets Demo  ==============================================================================================
        auto basic_id = uniqueId++;
        ed::BeginNode(basic_id);
        ImGui::Text("Basic Widget Demo");
        ed::BeginPin(uniqueId++, ed::PinKind::Input);
        ImGui::Text("-> In");
        ed::EndPin();
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(250, 0)); // Hacky magic number to space out the output pin.
        ImGui::SameLine();
        ed::BeginPin(uniqueId++, ed::PinKind::Output);
        ImGui::Text("Out ->");
        ed::EndPin();

        // Widget Demo from imgui_demo.cpp...
        // Normal Button
        static int clicked = 0;
        if (ImGui::Button("Button"))
            clicked++;
        if (clicked & 1)
        {
            ImGui::SameLine();
            ImGui::Text("Thanks for clicking me!");
        }

        // Checkbox
        static bool check = true;
        ImGui::Checkbox("checkbox", &check);

        // Radio buttons
        static int e = 0;
        ImGui::RadioButton("radio a", &e, 0);
        ImGui::SameLine();
        ImGui::RadioButton("radio b", &e, 1);
        ImGui::SameLine();
        ImGui::RadioButton("radio c", &e, 2);

        // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
                ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
            ImGui::Button("Click");
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }

        // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Hold to repeat:");
        ImGui::SameLine();

        // Arrow buttons with Repeater
        static int counter = 0;
        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left))
        {
            counter--;
        }
        ImGui::SameLine(0.0f, spacing);
        if (ImGui::ArrowButton("##right", ImGuiDir_Right))
        {
            counter++;
        }
        ImGui::PopButtonRepeat();
        ImGui::SameLine();
        ImGui::Text("%d", counter);

        // The input widgets also require you to manually disable the editor shortcuts so the view doesn't fly around.
        // (note that this is a per-frame setting, so it disables it for all text boxes.  I left it here so you could find it!)
        ed::EnableShortcuts(!io.WantTextInput);
        // The input widgets require some guidance on their widths, or else they're very large. (note matching pop at the end).
        ImGui::PushItemWidth(200);
        static char str1[128] = "";
        ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", str1, IM_ARRAYSIZE(str1));

        static float f0 = 0.001f;
        ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

        static float f1 = 1.00f, f2 = 0.0067f;
        ImGui::DragFloat("drag float", &f1, 0.005f);
        ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
        ImGui::PopItemWidth();

        ed::EndNode();
        if (firstframe)
        {
            ed::SetNodePosition(basic_id, ImVec2(20, 20));
        }

        // Headers and Trees Demo =======================================================================================================
        // TreeNodes and Headers streatch to the entire remaining work area. To put them in nodes what we need to do is to tell
        // ImGui out work area is shorter. We can achieve that right now only by using columns API.
        //
        // Relevent bugs: https://github.com/thedmd/imgui-node-editor/issues/30
        auto header_id = uniqueId++;
        ed::BeginNode(header_id);
        ImGui::Text("Tree Widget Demo");

        // Pins Row
        ed::BeginPin(uniqueId++, ed::PinKind::Input);
        ImGui::Text("-> In");
        ed::EndPin();
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(35, 0)); //  magic number - Crude & simple way to nudge over the output pin. Consider using layout and springs
        ImGui::SameLine();
        ed::BeginPin(uniqueId++, ed::PinKind::Output);
        ImGui::Text("Out ->");
        ed::EndPin();

        // Tree column startup -------------------------------------------------------------------
        // Push dummy widget to extend node size. Columns do not do that.
        float width = 135; // bad magic numbers. used to define width of tree widget
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(width, 0));
        ImGui::PopStyleVar();

        // Start columns, but use only first one.
        ImGui::BeginColumns("##TreeColumns", 2,
                            ImGuiOldColumnFlags_NoBorder |
                                ImGuiOldColumnFlags_NoResize |
                                ImGuiOldColumnFlags_NoPreserveWidths |
                                ImGuiOldColumnFlags_NoForceWithinWindow);

        // Adjust column width to match requested one.
        ImGui::SetColumnWidth(0, width + ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemSpacing.x);
        // End of tree column startup --------------------------------------------------------------

        // Back to normal ImGui drawing, in our column.
        if (ImGui::CollapsingHeader("Open Header"))
        {
            ImGui::Text("Hello There");
            if (ImGui::TreeNode("Open Tree"))
            {
                static bool OP1_Bool = false;
                ImGui::Text("Checked: %s", OP1_Bool ? "true" : "false");
                ImGui::Checkbox("Option 1", &OP1_Bool);
                ImGui::TreePop();
            }
        }
        // Tree Column Shutdown
        ImGui::EndColumns();
        ed::EndNode(); // End of Tree Node Demo

        if (firstframe)
        {
            ed::SetNodePosition(header_id, ImVec2(420, 20));
        }

        // Tool Tip & Pop-up Demo =====================================================================================
        // Tooltips, combo-boxes, drop-down menus need to use a work-around to place the "overlay window" in the canvas.
        // To do this, we must defer the popup calls until after we're done drawing the node material.
        //
        // Relevent bugs:  https://github.com/thedmd/imgui-node-editor/issues/48
        auto popup_id = uniqueId++;
        ed::BeginNode(popup_id);
        ImGui::Text("Tool Tip & Pop-up Demo");
        ed::BeginPin(uniqueId++, ed::PinKind::Input);
        ImGui::Text("-> In");
        ed::EndPin();
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(85, 0)); // Hacky magic number to space out the output pin.
        ImGui::SameLine();
        ed::BeginPin(uniqueId++, ed::PinKind::Output);
        ImGui::Text("Out ->");
        ed::EndPin();

        // Tooltip example
        ImGui::Text("Hover over me");
        static bool do_tooltip = false;
        do_tooltip = ImGui::IsItemHovered() ? true : false;
        ImGui::SameLine();
        ImGui::Text("- or me");
        static bool do_adv_tooltip = false;
        do_adv_tooltip = ImGui::IsItemHovered() ? true : false;

        // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements
        // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Option:");
        ImGui::SameLine();
        static char popup_text[128] = "Pick one!";
        static bool do_popup = false;
        if (ImGui::Button(popup_text))
        {
            do_popup = true; // Instead of saying OpenPopup() here, we set this bool, which is used later in the Deferred Pop-up Section
        }
        ed::EndNode();
        if (firstframe)
        {
            ed::SetNodePosition(popup_id, ImVec2(610, 20));
        }

        // --------------------------------------------------------------------------------------------------
        // Deferred Pop-up Section

        // This entire section needs to be bounded by Suspend/Resume!  These calls pop us out of "node canvas coordinates"
        // and draw the popups in a reasonable screen location.
        ed::Suspend();
        // There is some stately stuff happening here.  You call "open popup" exactly once, and this
        // causes it to stick open for many frames until the user makes a selection in the popup, or clicks off to dismiss.
        // More importantly, this is done inside Suspend(), so it loads the popup with the correct screen coordinates!
        if (do_popup)
        {
            ImGui::OpenPopup("popup_button"); // Cause openpopup to stick open.
            do_popup = false;                 // disable bool so that if we click off the popup, it doesn't open the next frame.
        }

        // This is the actual popup Gui drawing section.
        if (ImGui::BeginPopup("popup_button"))
        {
            // Note: if it weren't for the child window, we would have to PushItemWidth() here to avoid a crash!
            ImGui::TextDisabled("Pick One:");
            ImGui::BeginChild("popup_scroller", ImVec2(100, 100), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            if (ImGui::Button("Option 1"))
            {
                portable_strcpy(popup_text, "Option 1");
                ImGui::CloseCurrentPopup(); // These calls revoke the popup open state, which was set by OpenPopup above.
            }
            if (ImGui::Button("Option 2"))
            {
                portable_strcpy(popup_text, "Option 2");
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Option 3"))
            {
                portable_strcpy(popup_text, "Option 3");
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Option 4"))
            {
                portable_strcpy(popup_text, "Option 4");
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndChild();
            ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
        }

        // Handle the simple tooltip
        if (do_tooltip)
            ImGui::SetTooltip("I am a tooltip");

        // Handle the advanced tooltip
        if (do_adv_tooltip)
        {
            ImGui::BeginTooltip();
            ImGui::Text("I am a fancy tooltip");
            static float arr[] = {0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f};
            ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
            ImGui::EndTooltip();
        }

        ed::Resume();
        // End of "Deferred Pop-up section"

        // Plot Widgets =========================================================================================
        // Note: most of these plots can't be used in nodes missing, because they spawn tooltips automatically,
        // so we can't trap them in our deferred pop-up mechanism.  This causes them to fly into a random screen
        // location.
        auto plot_id = uniqueId++;
        ed::BeginNode(plot_id);
        ImGui::Text("Plot Demo");
        ed::BeginPin(uniqueId++, ed::PinKind::Input);
        ImGui::Text("-> In");
        ed::EndPin();
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(250, 0)); // Hacky magic number to space out the output pin.
        ImGui::SameLine();
        ed::BeginPin(uniqueId++, ed::PinKind::Output);
        ImGui::Text("Out ->");
        ed::EndPin();

        ImGui::PushItemWidth(300);

        // Animate a simple progress bar
        static float progress = 0.0f, progress_dir = 1.0f;
        progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
        if (progress >= +1.1f)
        {
            progress = +1.1f;
            progress_dir *= -1.0f;
        }
        if (progress <= -0.1f)
        {
            progress = -0.1f;
            progress_dir *= -1.0f;
        }

        // Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
        // or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("Progress Bar");

        float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f
                                                                                : progress;
        char buf[32];
        portable_sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
        ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);

        ImGui::PopItemWidth();
        ed::EndNode();
        if (firstframe)
        {
            ed::SetNodePosition(plot_id, ImVec2(850, 20));
        }
        // ==================================================================================================
        // Link Drawing Section

        for (auto &linkInfo : m_Links)
            ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

        // ==================================================================================================
        // Interaction Handling Section
        // This was coppied from BasicInteration.cpp. See that file for commented code.

        // Handle creation action ---------------------------------------------------------------------------
        if (ed::BeginCreate())
        {
            ed::PinId inputPinId, outputPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
            {
                if (inputPinId && outputPinId)
                {
                    if (ed::AcceptNewItem())
                    {
                        m_Links.push_back({ed::LinkId(m_NextLinkId++), inputPinId, outputPinId});
                        struct TreeNode *prevNode = (struct TreeNode *)((uint64_t)inputPinId + 1);
                        struct TreeNode *nextNode = (struct TreeNode *)((uint64_t)outputPinId - 1);
                        ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
                    }
                }
            }
        }
        ed::EndCreate();

        // Handle deletion action ---------------------------------------------------------------------------
        if (ed::BeginDelete())
        {
            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
                if (ed::AcceptDeletedItem())
                {
                    for (auto &link : m_Links)
                    {
                        if (link.Id == deletedLinkId)
                        {
                            m_Links.erase(&link);
                            break;
                        }
                    }
                }
            }
        }
        ed::EndDelete();

        ed::End();
        ed::SetCurrentEditor(nullptr);
        firstframe = false;
        // ImGui::ShowMetricsWindow();
        // ImGui::ShowDemoWindow();
    }

    ed::EditorContext *m_Context = nullptr;

    ImVector<LinkInfo> m_Links; // List of live links. It is dynamic unless you want to create read-only view over nodes.
    int m_NextLinkId = 100;     // Counter to help generate link ids. In real application this will probably based on pointer to user data structure.
};
