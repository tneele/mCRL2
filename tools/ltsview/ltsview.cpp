// Author(s): Bas Ploeger and Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ltsview.cpp
/// \brief Contains implementation of the LTSView application.

#include "wx.hpp" // precompiled headers

#define NAME "ltsview"
#define AUTHOR "Bas Ploeger and Carst Tankink"

#include "ltsview.h"
#include <string>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/image.h>
#include "mcrl2/lts/lts.h"
#include "mcrl2/core/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h"
#include "cluster.h"
#include "glcanvas.h"
#include "lts.h"
#include "mainframe.h"
#include "markmanager.h"
#include "markstateruledialog.h"
#include "settings.h"
#include "state.h"
#include "utils.h"
#include "visualizer.h"

using namespace std;
using namespace Utils;

#ifdef ENABLE_SQUADT_CONNECTIVITY
// Configures tool capabilities.
void LTSView::set_capabilities(tipi::tool::capabilities& c) const {
  std::set< mcrl2::lts::lts_type > const& input_formats(mcrl2::lts::lts::supported_lts_formats());

  for (std::set< mcrl2::lts::lts_type >::const_iterator i = input_formats.begin(); i != input_formats.end(); ++i) {
    c.add_input_configuration("main-input", tipi::mime_type(mcrl2::lts::lts::mime_type_for_type(*i)), tipi::tool::category::visualisation);
  }
}

// Queries the user via SQuADt if needed to obtain configuration information
void LTSView::user_interactive_configuration(tipi::configuration&) { }

// Check an existing configuration object to see if it is usable
bool LTSView::check_configuration(tipi::configuration const& c) const {
  if (c.input_exists("main-input")) {
    /* The input object is present, verify whether the specified format is supported */
    if (mcrl2::lts::lts::parse_format(c.get_input("main-input").type().sub_type().c_str()) == mcrl2::lts::lts_none) {
      send_error("Invalid configuration: unsupported type `" +
          c.get_input("main-input").type().sub_type() + "' for main input");
    }
    else {
      return true;
    }
  }

  return false;
}

bool LTSView::perform_task(tipi::configuration& c) {
  m_input_filename = c.get_input("main-input").location();

  return super::perform_task(c);
}
#endif

std::vector< std::string > developers()
{
  static char const* developer_names[] = {"Bas Ploeger", "Carst Tankink"};
  return std::vector< std::string >(&developer_names[0], &developer_names[2]);
}


LTSView::LTSView() : super("LTSView",
    "3D interactive visualization of a labelled transition system", // what-is
    "Tool for interactive visualization of state transition systems.\n" // gui-specific description
    "\n"
    "LTSView is based on visualization techniques by Frank van Ham and Jack van Wijk.\n"
    "See: F. van Ham, H. van de Wetering and J.J. van Wijk,\n"
    "\"Visualization of State Transition Graphs\". "
    "Proceedings of the IEEE Symposium on Information Visualization 2001. IEEE CS Press, pp. 59-66, 2001.\n"
    "\n"
    "The default colour scheme for state marking was obtained from http://www.colorbrewer.org",
    "Start the LTSView application. If INFILE is supplied then the " // command-line description
    "LTS in INFILE is loaded into the application.\n"
    "\n"
    "The input format is determined by the contents of INFILE. If that fails, "
    "an attempt is made to force the input format based on the file extension. "
    "The supported formats with file extensions are:\n"
    "  Aldebaran format (CADP; *.aut);\n"
#ifdef USE_BCG
    "  Binary Coded Graph format (CADP; *.bcg);\n"
#endif
    "  GraphViz format (*.dot);\n"
    "  Finite State Machine format (*.fsm);\n"
    "  mCRL SVC format (*.svc);\n"
    "  mCRL2 format (*.lts).",
    developers())
{ }

IMPLEMENT_APP_NO_MAIN(LTSView)
IMPLEMENT_WX_THEME_SUPPORT
BEGIN_EVENT_TABLE(LTSView, wxApp)
END_EVENT_TABLE()

#ifdef __WINDOWS__
extern "C" int WINAPI WinMain(HINSTANCE hInstance,
                                  HINSTANCE hPrevInstance,
                                  wxCmdLineArgType lpCmdLine,
                                  int nCmdShow) {

  MCRL2_ATERMPP_INIT(0, lpCmdLine)

  return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#else
int main(int argc, char **argv)
{
  MCRL2_ATERMPP_INIT(argc, argv)

  return wxEntry(argc, argv);
}
#endif

bool LTSView::run()
{
  rankStyle = ITERATIVE;
  fsmStyle = false;
  colourCounter = 0;
  settings = new Settings();
  mainFrame = new MainFrame(this,settings);
  visualizer = new Visualizer(this,settings);
  markManager = new MarkManager();
  glCanvas = mainFrame->getGLCanvas();
  glCanvas->setVisualizer(visualizer);

  SetTopWindow(mainFrame);
  mainFrame->Show(true);
  glCanvas->initialize();
  mainFrame->Layout();

  wxInitAllImageHandlers();

  if (!input_filename().empty())
  {
    wxFileName fileName(wxString(input_filename().c_str(), wxConvLocal));
    fileName.Normalize();
    mainFrame->setFileInfo(fileName);
    openFile(static_cast< string >(fileName.GetFullPath().fn_str()));
  }
  return true;
}

int LTSView::OnExit()
{
  if (lts != 0)
  {
    delete lts;
    delete settings;
    delete visualizer;
    delete markManager;
  }
  return 0;
}

std::string LTSView::getVersionString()
{
  return mcrl2::utilities::interface_description("", NAME, AUTHOR, "",
      "", "").version_information();
}

void LTSView::openFile(string fileName)
{
  glCanvas->disableDisplay();

  mainFrame->createProgressDialog("Opening file","Loading file");
  mainFrame->updateProgressDialog(0,"Loading file");
  LTS* newlts = new LTS(this);
  if (!newlts->readFromFile(fileName))
  {
    delete newlts;
    mainFrame->updateProgressDialog(100,"Error loading file");
    mainFrame->showMessage(
        "Error loading file",
        "Could not load file " + fileName);
    return;
  }
  delete lts;
  lts = newlts;

  mainFrame->updateProgressDialog(17,"Ranking states");
  lts->rankStates(rankStyle);

  mainFrame->updateProgressDialog(33,"Clustering states");
  lts->clusterStates(rankStyle);

  mainFrame->updateProgressDialog(50,"Setting cluster info");
  lts->computeClusterInfo();

  mainFrame->updateProgressDialog(67,"Positioning clusters");
  lts->positionClusters(fsmStyle);

  markManager->setLTS(lts,true);
  visualizer->setLTS(lts,true);

  mainFrame->updateProgressDialog(83,"Positioning states");
  lts->positionStates();

  mainFrame->updateProgressDialog(100,"Done");
  visualizer->notifyMarkStyleChanged();

  glCanvas->enableDisplay();
  glCanvas->resetView();

  mainFrame->loadTitle();
  mainFrame->setNumberInfo(lts->getNumStates(),
      lts->getNumTransitions(),lts->getNumClusters(),
      lts->getNumRanks());
  mainFrame->resetParameters();
  for (unsigned int i = 0; i < lts->getNumParameters(); ++i) {
    mainFrame->addParameter(i,lts->getParameterName(i));
  }
  mainFrame->resetMarkRules();

  vector< string > ls;
  lts->getActionLabels(ls);

  mainFrame->setSim(lts->getSimulation());
  mainFrame->setActionLabels(ls);

  glCanvas->setSim(lts->getSimulation());

  mainFrame->setMarkedStatesInfo(0);
  mainFrame->setMarkedTransitionsInfo(0);
}

void LTSView::setRankStyle(RankStyle rs)
{
  if (rankStyle != rs)
  {
    rankStyle = rs;
    if (lts != NULL)
    {
      glCanvas->disableDisplay();

      mainFrame->createProgressDialog("Structuring LTS","Applying ranking");

      mainFrame->updateProgressDialog(17,"Ranking states");
      lts->rankStates(rankStyle);

      mainFrame->updateProgressDialog(33,"Clustering states");
      lts->clusterStates(rankStyle);

      mainFrame->updateProgressDialog(40,"Setting cluster info");
      lts->computeClusterInfo();
      markManager->markClusters();

      mainFrame->updateProgressDialog(60,"Positioning clusters");
      lts->positionClusters(fsmStyle);

      visualizer->setLTS(lts,true);

      mainFrame->updateProgressDialog(80,"Positioning states");
      lts->positionStates();

      mainFrame->updateProgressDialog(100,"Done");

      glCanvas->enableDisplay();
      glCanvas->resetView();

      mainFrame->setNumberInfo(lts->getNumStates(),
        lts->getNumTransitions(),lts->getNumClusters(),
        lts->getNumRanks());
    }
  }
}

void LTSView::setVisStyle(VisStyle vs)
{
  if (visualizer->getVisStyle() != vs)
  {
    visualizer->setVisStyle(vs);
    glCanvas->display();
  }
}

void LTSView::setFSMStyle(bool b)
{
  if (b != fsmStyle)
  {
    fsmStyle = b;
    if (lts != NULL)
    {
      glCanvas->disableDisplay();
      lts->positionClusters(fsmStyle);
      visualizer->setLTS(lts,true);
      glCanvas->enableDisplay();
      glCanvas->resetView();
    }
  }
}

void LTSView::addMarkRule()
{
  if (lts != NULL)
  {
    MarkStateRuleDialog* msrdlg = new MarkStateRuleDialog(mainFrame,this,lts);
    msrdlg->CentreOnParent();
    if (msrdlg->ShowModal() == wxID_OK)
    {
      if (msrdlg->getParamIndex() != -1)
      {

        int mr = markManager->createMarkRule(msrdlg->getParamIndex(),
            msrdlg->getNegated(),msrdlg->getColor(),
            msrdlg->getValues());
        mainFrame->addMarkRule(msrdlg->getMarkRuleString(),mr);

        if (markManager->getMarkStyle() != MARK_STATES)
        {
          setMarkStyle(MARK_STATES);
        }
        else
        {
          applyMarkStyle();
        }
      }
    }
    msrdlg->Close();
    msrdlg->Destroy();
  }
}

void LTSView::removeMarkRule(int mr)
{
  markManager->removeMarkRule(mr);
  if (markManager->getMarkStyle() != MARK_STATES)
  {
    setMarkStyle(MARK_STATES);
  }
  else
  {
    applyMarkStyle();
  }
}

void LTSView::editMarkRule(int mr)
{
  if (lts != NULL)
  {
    MarkStateRuleDialog* msrdlg = new MarkStateRuleDialog(mainFrame,
        this,lts);
    msrdlg->setData(markManager->getMarkRuleParam(mr),
        markManager->getMarkRuleColor(mr),
        markManager->getMarkRuleNegated(mr),
        markManager->getMarkRuleValues(mr));
    msrdlg->CentreOnParent();
    if (msrdlg->ShowModal() == wxID_OK)
    {
      if (msrdlg->getParamIndex() != -1)
      {
        markManager->setMarkRuleData(mr,msrdlg->getParamIndex(),
            msrdlg->getNegated(),msrdlg->getColor(),
            msrdlg->getValues());
        mainFrame->replaceMarkRule(msrdlg->getMarkRuleString(),mr);
        if (markManager->getMarkStyle() != MARK_STATES)
        {
          setMarkStyle(MARK_STATES);
        }
        else
        {
          applyMarkStyle();
        }
      }
    }
    msrdlg->Close();
    msrdlg->Destroy();
  }
}

Utils::MarkStyle LTSView::getMarkStyle()
{
  return markManager->getMarkStyle();
}

Utils::MatchStyle LTSView::getMatchStyle()
{
  return markManager->getMatchStyle();
}

bool LTSView::isMarked(State* s)
{
  return markManager->isMarked(s);
}

bool LTSView::isMarked(Cluster* c)
{
  return markManager->isMarked(c);
}

bool LTSView::isMarked(Transition* t)
{
  return markManager->isMarked(t);
}

Utils::RGB_Color LTSView::getMarkRuleColor(int mr)
{
  return markManager->getMarkRuleColor(mr);
}

Utils::RGB_Color LTSView::getNewRuleColour()
{
  // TODO implement
  colourCounter = (colourCounter + 1) % 9;

  RGB_Color result;

  switch(colourCounter)
  {
    case 0:
      { result.r = 228;
        result.g = 26;
        result.b = 28;
        break;
      }
    case 1:
      { result.r = 55;
        result.g = 126;
        result.b = 184;
        break;
      }
    case 2:
      { result.r = 77;
        result.g = 175;
        result.b = 74;
        break;
      }
    case 3:
      { result.r = 152;
        result.g = 78;
        result.b = 163;
        break;
      }
    case 4:
      { result.r = 255;
        result.g = 127;
        result.b = 0;
        break;
      }
    case 5:
      { result.r = 255;
        result.g = 255;
        result.b = 51;
        break;
      }
    case 6:
      { result.r = 166;
        result.g = 86;
        result.b = 40;
        break;
      }
    case 7:
     { result.r = 247;
       result.g = 129;
       result.b = 191;
       break;
     }
    case 8:
     { result.r = 153;
       result.g = 153;
       result.b = 153;
       break;
     }
    default: //does not occur for modulo, but keep compiler happy
    {  result.r = 0;
       result.g = 0;
       result.b = 0;
    }
  }
  return result;
}

void LTSView::activateMarkRule(int mr,bool activate)
{
  if (lts != NULL)
  {
    markManager->setMarkRuleActivated(mr,activate);
    if (markManager->getMarkStyle() != MARK_STATES)
    {
      setMarkStyle(MARK_STATES);
    }
    else
    {
      applyMarkStyle();
    }
  }
}

void LTSView::setMatchStyle(Utils::MatchStyle ms)
{
  if (lts == NULL) return;
  markManager->setMatchStyle(ms);
  if (markManager->getMarkStyle() != MARK_STATES)
  {
    setMarkStyle(MARK_STATES);
  }
  else
  {
    applyMarkStyle();
  }
}

void LTSView::setMatchStyleClusters(Utils::MatchStyle ms)
{
  if (lts == NULL) return;
  markManager->setMatchStyleClusters(ms);
  applyMarkStyle();
}

void LTSView::setActionMark(int l,bool b)
{
  markManager->setActionMark(l,b);
  setMarkStyle(MARK_TRANSITIONS);
}

void LTSView::setMarkStyle(Utils::MarkStyle ms)
{
  markManager->setMarkStyle(ms);
  applyMarkStyle();
}

void LTSView::applyMarkStyle()
{
  if (lts == NULL) return;

  switch (markManager->getMarkStyle())
  {
    case MARK_DEADLOCKS:
      mainFrame->setMarkedStatesInfo(lts->getNumDeadlocks());
      mainFrame->setMarkedTransitionsInfo(0);
      break;
    case MARK_STATES:
      mainFrame->setMarkedStatesInfo(markManager->getNumMarkedStates());
      mainFrame->setMarkedTransitionsInfo(0);
      break;
    case MARK_TRANSITIONS:
      mainFrame->setMarkedStatesInfo(0);
      mainFrame->setMarkedTransitionsInfo(
          markManager->getNumMarkedTransitions());
      break;
    case NO_MARKS:
    default:
      mainFrame->setMarkedStatesInfo(0);
      mainFrame->setMarkedTransitionsInfo(0);
      break;
  }
  visualizer->notifyMarkStyleChanged();
  glCanvas->display();
}

void LTSView::notifyRenderingStarted()
{
  mainFrame->startRendering();
}

void LTSView::notifyRenderingFinished()
{
  mainFrame->stopRendering();
}

void LTSView::startSim()
{
  Simulation* sim = lts->getSimulation();

  sim->start();

  // Select/deselect initial state of simulation, to initialize state info pane.
  selectStateByID(sim->getCurrState()->getID());
}

int LTSView::getNumberOfParams() const
{
  if (lts != NULL)
  {
    return lts->getNumParameters();
  }
  else
  {
    return 0;
  }
}
string LTSView::getActionLabel(const int i) const
{
  if (lts != NULL)
  {
    return lts->getLabel(i);
  }
  else
  {
    return "";
  }
}

string LTSView::getParName(const int i) const
{
  if (lts != NULL)
  {
    return lts->getParameterName(i);
  }
  else
  {
    return "";
  }
}

string LTSView::getParValue(State *s, const int j) const
{
  if (lts != NULL)
  {
    return lts->getStateParameterValueStr(s,j);
  }
  else
  {
    return "";
  }
}


void LTSView::selectStateByID(const int id)
{
  if (lts != NULL)
  {
    State* s = lts->selectStateByID(id);
    lts->getSimulation()->setInitialState(s);
    for (unsigned int i = 0; i < lts->getNumParameters(); ++i)
    {
      mainFrame->setParameterValue(i,
          lts->getStateParameterValueStr(s,i));
    }
  }
}

void LTSView::selectCluster(const int rank, const int pos)
{
  if (lts != NULL)
  {
    Cluster* c = lts->selectCluster(rank, pos);
    mainFrame->setClusterStateNr(c->getNumStates());
    for (unsigned int i = 0; i < lts->getNumParameters(); ++i)
    {
      atermpp::set<ATerm> values = lts->getClusterParameterValues(c,i);
      atermpp::set<ATerm>::iterator val_it;
      std::vector<std::string> val;
      for (val_it = values.begin(); val_it != values.end(); ++val_it)
      {
        val.push_back(lts->prettyPrintParameterValue(*val_it));
      }
      mainFrame->setParameterValues(i, val);
    }
  }
}

void LTSView::deselect()
{
  if (lts != NULL)
  {
    lts->deselect();

    mainFrame->resetParameterValues();
  }
}

int LTSView::getNumberOfObjects()
{
  int result = 0;

  if (lts != NULL)
  {
    result += lts->getNumClusters();
    result += lts->getNumStates();
  }

  return result;
}

void LTSView::zoomInBelow()
{
  LTS* newLTS = lts->zoomIntoBelow();
  deselect();
  lts = newLTS;
  markManager->setLTS(lts,false);
  visualizer->setLTS(lts,false);
  mainFrame->setNumberInfo(lts->getNumStates(),
      lts->getNumTransitions(),lts->getNumClusters(),
      lts->getNumRanks());
  applyMarkStyle();
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());
}

void LTSView::zoomInAbove()
{
  LTS* newLTS = lts->zoomIntoAbove();
  deselect();
  lts = newLTS;
  markManager->setLTS(lts,false);
  visualizer->setLTS(lts,false);
  mainFrame->setNumberInfo(lts->getNumStates(),
     lts->getNumTransitions(),lts->getNumClusters(),
     lts->getNumRanks());
  applyMarkStyle();
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());
}

void LTSView::zoomOutTillTop()
{
  LTS* oldLTS = lts;
  do
  {
    zoomOut();
    oldLTS = lts;
  }  while (lts != oldLTS);
}

void LTSView::zoomOut()
{
  LTS* oldLTS = lts;
  lts = oldLTS->zoomOut();
  oldLTS->deselect();
  markManager->setLTS(lts,false);
  visualizer->setLTS(lts,false);
  mainFrame->setNumberInfo(lts->getNumStates(),
    lts->getNumTransitions(),lts->getNumClusters(),
    lts->getNumRanks());
  applyMarkStyle();
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());

  if (oldLTS != lts)
  {
    delete oldLTS;
  }
}

void LTSView::loadTrace(std::string const& path)
{
  lts->loadTrace(path);
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());
}

void LTSView::reportError(std::string const& error)
{
  mainFrame->reportError(error);
}


void LTSView::generateBackTrace()
{
  lts->generateBackTrace();
}

void LTSView::exportToText(std::string filename)
{
  visualizer->exportToText(filename);
}
