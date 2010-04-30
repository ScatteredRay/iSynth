import re
import sys

class Definition:
  def __init__(_):
    _.state = None
    _.indentation = 0
    _.parent_indent = 0

    _.name = None
    _.stereo = False
    _.parameters = []
    _.fill = "" 
    _.misc = ""
    _.output_range = ""
    _.initialize = ""
    _.members = []
    _.input_range = []
  
  def saveTranslation(_):
    print """
class %s : public %sModule
{
  public:
    %s(vector<ModuleParam *> parameters)
    {""" % (_.name, _.stereo, _.name)

    n = 0
    for p in _.parameters:
      print "      m_%s = parameters[%d]->m_%s;" % (p[0], n, p[1].lower())
      n += 1

    for m in _.members: print "      m_%s = %s;" % (m[0], m[2])
    print _.initialize
    print """    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new %s(parameters);
    }
""" % _.name
    
    print "    const char *moduleName() { return \"%s\"; }" % _.name
    
    print _.misc
    
    print """    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;"""
    for p in _.parameters:
      if p[1].endswith("Module"):
        print "      const float *%s = m_%s->output(last_fill, samples);" % \
              (p[0], p[0])
    print """
      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
%s      }
      g_profiler.addTime("%s", hires_time()-time);
    }""" % (_.fill, _.name)
    
    print """
    void getOutputRange(float *out_min, float *out_max)
    {
%s    }

    void validateInputRange()
    {""" % (_.output_range)
    for i in _.input_range:
      print "      validateWithin(*m_%s, %s, %s);" % i
    print "    }"
    
    print "  private:"
    for p in _.parameters:
      type = p[1]+" *" if p[1].endswith("Module") else p[1]+" "
      print "    %sm_%s;" % (type, p[0])
    for m in _.members: print "    %s m_%s;" % (m[1], m[0])
    print "};"
  
  def parseModuleLine(_, line):
    m = re.match("(stereo)?module (.+)\((.*)\):", line)
    if not m: raise "expected module definition in "+line
    stereo, _.name, params = m.groups()
    if stereo: _.stereo = "Stereo"
    else:      _.stereo = ""
    for p in re.split(",\s+", params):
      tp = p.split(" ")
      name = tp[0]
      type = "Module"
      if len(tp) > 1: type, name = tp
      default = None
      name_default = name.split("=")
      if len(name_default) > 1: name, default = name_default
      if name!="": _.parameters.append((name, type, default))
  
  def parseMemberLine(_, line):    
    line = line.strip()
    space = line.find(" ")
    assert space > -1
    type = line[:space]
    for m in line[space+1:].split(","):
      member = m.split("=")
      name = member[0].strip()
      value = member[1].strip() if len(member) > 1 else None
      _.members.append((name, type, value))

  def parseFillLine       (_, line): _.fill         += "    " + line + "\n"
  def parseMiscLine       (_, line): _.misc         +=          line + "\n"
  def parseOutputRangeLine(_, line): _.output_range += "  "   + line + "\n"
  def parseInitializeLine (_, line): _.initialize   += "  "   + line + "\n"
    
  def parseInputRangeLine(_, line):
    name, params = line.split(":")
    params = params.split(",")
    _.input_range.append((name.strip(), params[0].strip(), params[1].strip()))
    
  def parseLine(_, line):
    if len(line.strip()) == 0: return
    _.indent = len(line) - len(line.lstrip())
    if _.parent_indent == 0: _.parent_indent = _.indent
  
    if _.indent == _.parent_indent:
      _.state = None
      _.indentation    
    
    if line.startswith("module") or line.startswith("stereomodule"):
      _.parseModuleLine(line)
      return
    if _.state == None and line.endswith(":"):
      _.state = line[:-1].lstrip()
      return
    
    if   _.state == "members":      _.parseMemberLine     (line)
    elif _.state == "fill":         _.parseFillLine       (line)
    elif _.state == "initialize":   _.parseInitializeLine (line)
    elif _.state == "output_range": _.parseOutputRangeLine(line)
    elif _.state == "input_range":  _.parseInputRangeLine (line)
    elif _.state == "misc":         _.parseMiscLine       (line)
    else:
      raise "Didn't know what to do with " + line + " in state: " + str(_.state)

def writeModuleMetadata(modules):
  print """void fillModuleList()
{""";
  for m in modules:
    print "  g_module_infos[\"%s\"] = new ModuleInfo(\"%s\", %s::create);" \
          % (m.name, m.name, m.name)
    for p in m.parameters:
      print "  g_module_infos[\"%s\"]->addParameter(\"%s\", \"%s\"%s);" % \
            (m.name, p[0], p[1], ", %s"%p[2] if p[2] else "")
  print "}"

def main():
  print "// Generated code! Poke around in modules.pre and " + \
        "moduleprocess.py, not here."
  module_list = []

  definition = None
  for line in sys.stdin.xreadlines():
    if definition == None:
      if line[0] == "@":
        line = line[1:]
        definition = Definition()
      else:
        print line.rstrip()
    if definition != None:
      if line[0] == "@":
        definition.saveTranslation()
        module_list.append(definition)
        definition = None
      else:
        definition.parseLine(line.rstrip())
  
  writeModuleMetadata(module_list)  
  
if __name__ == "__main__": main()